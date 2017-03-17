#include "ttest.h"

#include <algorithm>
#include <iostream>
#include <sstream>
#include <string>
#include <vector>

using ttest::error_log;
using ttest::create_test;

void logging_test(error_log& log) {

  error_log test_log("test");

  test_log.append("1");
  test_log.append_if("2", false);
  test_log.append_if("3", true);
  test_log.append();


  std::stringstream ss;
  test_log.report(ss);

  std::string log_string {ss.str()};
  std::string expected {"test::1\ntest::3\ntest\n"};
  log.append_if("incorrect log", log_string != expected);
}

// This test creates a test hierarchy, which, when run, produces errors
// at all levels. We check the number of error messages and their content.
void hierarchy_test(error_log& log) {
  auto first = create_test("A",[](error_log& l){l.append();});
  auto second = create_test("B",[](error_log& l){l.append();});
  auto third = create_test("C",[](error_log& l){l.append();});

  auto compound = create_test("compound", {
        first,
        create_test("sub", {second, third})
      });

  std::stringstream ss;
  compound->run_test();

  log.append_if("error count", compound->error_count() != 3u);

  compound->report(ss);

  // Note that the order of the errors in the log is not uniquely determined.
  // So to compare the errors with the expected errors, we must first sort
  // the messages.
  std::vector<std::string> messages;
  for (std::string line; std::getline(ss, line); messages.push_back(line)) {}
  std::sort(messages.begin(), messages.end(), 
      [](const auto& l, const auto& r) {
        return l.back() < r.back();
      });

  std::vector<std::string> expected_messages {
    "compound::A", "compound::sub::B", "compound::sub::C"
  };
  log.append_if("mismatch", !std::equal(messages.begin(), messages.end(),
        expected_messages.begin(), expected_messages.end()));
}

int main() {
  auto my_test = create_test("ttest", {
      create_test("logging", logging_test),
      create_test("hierarchy", hierarchy_test)
  });


  my_test->run_test();
  auto error_count = my_test->error_count();
  my_test->report(std::cerr);

  if (error_count) {
    std::cout << "There were " << error_count << " errors\n";
  } else {
    std::cout << "Success.\n";
  }

  return 0;
}
