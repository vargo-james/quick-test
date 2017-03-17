#ifndef _ttest_h_
#define _ttest_h_

#include <algorithm>
#include <cstddef>
#include <initializer_list>
#include <functional>
#include <memory>
#include <mutex>
#include <ostream>
#include <string>
#include <utility>
#include <vector>

namespace ttest {

/*
 * error_log is thread_safe. However, if multiple logs are reporting to 
 * the same ostream from different threads, the messages could be interleaved.
 */

class error_log {
 public:
  using message_type = std::basic_string<char>;
  using ostream = std::basic_ostream<char>;

  error_log(message_type name): qualifying_name_ {std::move(name)} {}

  size_t size() const {return log_.size();}
  void report(ostream& os) const;

  void append(const message_type& msg); 
  void append() {append({});}
  void append_if(const message_type& msg, bool fail) {if (fail) append(msg);}
  void append_if(bool fail) {if (fail) append();}

  // This appends each message from the sublog into the log.
  void incorporate(const error_log& sublog);

 private:
  message_type qualifying_name_;
  std::vector<message_type> log_;
  std::mutex incorporate_mutex;
  std::mutex append_mutex;
};

inline void error_log::append(const message_type& msg) {
  std::lock_guard<std::mutex> lock(append_mutex);
  if (msg.empty()) {
    log_.push_back(qualifying_name_);
  } else {
    log_.push_back(qualifying_name_ + "::" + msg);
  }
}

// The lock here is not strictly necessary because the calls to append
// are already guarded. This lock does preserve the contiguity of tests
// from the same subtest.
inline void error_log::incorporate(const error_log& sublog) {
  std::lock_guard<std::mutex> lock(incorporate_mutex);
  for (const auto& msg: sublog.log_) {
    append(msg);
  }
}

inline void error_log::report(ostream& os) const {
  for (const auto& msg: log_) {
    os << msg << '\n';
  }
}

/*
 * The test_suite class is a mechanism for composing a group of tests into
 * a conglomerate which is itself a test. It has two derived classes:
 * simple_test, which is constructed from a single test function; and
 * compound_test, which is constructed from an initializer list of 
 * pointers to test_suite objects.
 */

/*
 * The following struct is used to isolate the introduction of shared_ptr 
 * to one place. Ideally, we would use unique_ptr since shared owndership
 * is not necessary. However, using shared_ptr keeps the code simple while 
 * allowing the following interface for creating a compound test:  
 *
 *  auto pointer_to_my_test = create_test("test_name", {
 *    create_test("subtest1", test1),
 *    create_test("subtest2", test2),
 *    create_submoduleA_test(),
 *    create_submoduleB_test(),
 *  });
 *
 *  A compound test is made up of a vector of pointers to subtests. This 
 *  notation initializes that vector with an initializer_list of pointers.
 *  Initializer_lists are always const and can only be iterated
 *  over with const iterators. They can only be copied from, never moved from. 
 *  This is a problem for unique_ptr. So I use shared_ptr instead as a hack.
 *  It avoids deep copying at the cost of maintaining reference counting.
 */

template <typename T>
struct make_pointer{
  using pointer = std::shared_ptr<T>;
  template <typename... Args>
  pointer operator()(Args&&... args) {
    return std::make_shared<T>(std::forward<Args>(args)...);
  }
};

class test_suite {
 public:
  using message_type = typename error_log::message_type;
  using ostream = typename error_log::ostream;
  using pointer = typename make_pointer<test_suite>::pointer;

  test_suite(message_type name)
    : errors_(std::move(name)) {}

  void run_test() {do_test();}

  void report(ostream& os) const {errors_.report(os);}
  std::size_t error_count() const {return errors_.size();}

  virtual ~test_suite() = default;
 protected:
  // This function would like to be a private method of the compound_test
  // class. However, it needs to access a protected method on separate
  // instances of test_suite objects. That is not directly possible.
  void collect_errors(const std::vector<pointer>& subtests);

  error_log& errors() {return errors_;}
 private:
  virtual void do_test() = 0; 

  error_log errors_;
};

inline void test_suite::collect_errors(const std::vector<pointer>& subtests) {
  std::for_each(subtests.begin(), subtests.end(), [this](pointer test) {
        test->run_test();
        this->errors_.incorporate(test->errors_);
      });
}

class compound_test : public test_suite {
 public:
  using test_suite::message_type;
  using test_suite::pointer;

  compound_test(message_type name, std::initializer_list<pointer> tests) 
    : test_suite(std::move(name)), 
      components(tests) {}

 private:
  std::vector<pointer> components;

  void do_test() override {this->collect_errors(components);}
};

inline test_suite::pointer create_test(compound_test::message_type name, 
    std::initializer_list<test_suite::pointer> test_list) {
  return make_pointer<compound_test>{}(std::move(name), test_list);
}

class simple_test : public test_suite {
 public:
  using test_suite::message_type;
  using test_type = std::function<void(error_log&)>;

  simple_test(message_type name, test_type test) 
    : test_suite(std::move(name)), 
      test_ {test} {}

 private:
  test_type test_;

  void do_test() override {test_(errors());}
};

template <typename F>
test_suite::pointer create_test(simple_test::message_type name, F test) {
  return make_pointer<simple_test>{}(std::move(name), test);
}

}//namespace ttest

#endif// _ttest_h_
