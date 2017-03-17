# Quick Tests

This is a simple, lightweight test framework for C++ programs. 
It is called quick because you can start using it in about five minutes.  
I made it for myself because I do not like writing tests and I do not like 
reading documentation. So the use of any other test framework has two strikes 
against it right away. I made this as easy as possible to use because I knew that
if I ever forgot how to use it, then I would probably never use it again.
To that end, I made this a header only library. There are no methods
with more than a few lines of code, so I have simply inlined every one. 

I think the interface has an easy learning curve, and I sincerely hope that
you agree. Of course that implies a certain 
lack of flexibility/functionality. But for my needs, and maybe yours, all that 
is necessary is to run a series of test functions and then get a report listing
any errors that were found. If you need more than that, or if you already know
and love a "real test framework", then you don't need this one.

## Features

  1. SIMPLE INTERFACE: You need to know the following things:
    a. `#include "ttest.h"`. The extra t stands for "tree".
    b. Test functions have the signature `void(ttest::error_log&)`.
    c. `log.append("error message")` OR `log.append_if("msg", failure)`.
    d. The `create_test` function.
    e. all_my_tests->run_test(); all_my_tests->report(std::cout);

    I usually forget what the last method calls are because I only write them
    once per project. But then I just look in my last project (or in the 
    test_suite class definition). You can just follow the examples here.

  2. HIERARCHICAL STRUCTURE: Tests are composed using the classic composite
    pattern. The actual class is `test_suite`. A test_suite object has the 
    structure of a tree. A leaf in this tree is a test_suite object that holds 
    a single test function. A proper node is a test_suite object that itself 
    holds a collection of test_suite objects. The benefit of this structure is 
    that it is as easy to define the test object for your entire 
    library/program as it is for a single class method.

  3. GOOD REPORTING: Error messages are "scoped" so that you can easily tell
    exactly which test function produced the message without having to somehow
    encode that information in the actual error message.

  4. THREAD SAFETY: The error_log is thread safe. So your test functions can
    have concurrent threads writing to the log. Also, the code has been written
    in such as way as to enable the concurrent processing of test_functions
    once C++17 becomes the standard. I could have enabled that already of 
    course, but for me, it wasn't worth the trouble. With C++17, I will simply
    add an execution policy to a single call of std::for_each. WARNING: The
    order in which tests are conducted is not specified. So your test functions
    should be self contained. Thread safe is best.

## Requirements

  This should work if your compiler supports C++11. I have only tested it
  on my own machine (Debian 'jessie' with GCC 6.3). I plan to use the new
  overload of std::for_each at some point after C++17 comes out, so that 
  will certainly break on some old compilers.

## A Brief Explanation

  Assume the following using declarations for all code below:

```c++
using namespace std;        // For the sake of brevity.
using ttest::test_suite;
using ttest::error_log;
using ttest::create_test;
```

  The extra t in the namespace stands for "tree". You will want to create
  test_suite objects using the create_test function. This function is
  overloaded to make simple test_suite objects containing just one test
  function, and compound test_suite objects that contain many test functions.

  You will be working with test_suite::pointer objects. This is just a 
  std::shared_ptr<test_suite>.

#### Test Functions

  To make tests you need to define test functions. A test function always has
  the signature

    void my_test_function(error_log& log);

  Inside that test function, if your test uncovers an error, it should append
  it to the log with an optional message.

    log.append("error!")
    log.append()

  If you call append with no message, the error report will still identify
  the exact test function which found the error.

  Most of the time, it is convenient to report errors this way:

    log.append_if("error!", something_bad_happened);
    log.append_if(something_else_bad_happened);

  The non-string arguments are boolean values. An error is only registered
  if its value is true.

#### Simple Tests

  To make a simple test_suite object that contains just one test function
  you will need to use the `create_test` function with the following signature:

```c++
test_suite::pointer create_test(string name, function<void(error_log&)> test);

auto my_simple_test = create_test("Foo",bar);
```

#### Compound Tests

  To make a compound test_suite object you use the `create_test` function with
  the following signature:

```c++
test_suite::pointer create_test(string name, 
    initializer_list<test_suite::pointer> subtests);

auto my_big_test = create_test("FooFoo", {
    create_test("Bar", testfun1),
    create_test("Baz", testfun2),
    create_test("42", testfun3)
  });
```

#### Reporting
  Reports are sent to a std::ostream.

```c++
auto my_test = create_test("a big ol' test", 
    {\* a list of test_suite::pointers*\});

my_test->run_test();
my_test->error_count();     // The number of log entries.
my_test->report(std::cout);
```

  In the report all messages are scoped using the names of the test_suite 
  objects. So for each error listed in the report, it is very easy to identify
  exactly which test function produced that error. See the detailed code
  example to see what the output looks like.
  

## A Detailed Code Example

The following code example should illustrate everything you need to know to 
use this test framework.

`myclass_tests.cpp`

```c++
#include "ttest.h"

using ttest::error_log;
using ttest::create_test;
using ttest::test_suite;

void myclass_testA(error_log& log) {

  bool first_test_failed = false;
  bool second_test_failed = true;

  // Test code here. Lets assume the booleans are unchanged.

  // Here is one way to register an error.
  if (first_test_failed) {          // It didn't fail.
    log.append("failure message");
  }

  // Here is another way. This test did fail, so this error gets registered.
  log.append_if("another failure message", second_test_failed);
}

void myclass_testB(error_log& log) {
  // Another test that registers errors with the log.
  // Lets assume no errors are found in this test.
}

// Now we collect all the tests for myclass into a single test_suite.
// It is convenient to make a function rather than a global object.
test_suite::pointer create_myclass_test() {
  return create_test("myclass", {
    create_test("testA", myclass_testA),
    create_test("testB", myclass_testB)
  });
}
```

So we might have a few other files with similar class tests in them. We can
collect them together in the file `mymodule_tests.cpp`.

```c++
#include "ttest.h"

using ttest::error_log;
using ttest::create_test;
using ttest::test_suite;

// Declare the various unit tests.
test_suite::pointer create_myclass_test();
test_suite::pointer create_my_other_class_test();
test_suite::pointer create_my_favorite_class_test();

// Maybe we want a special test function that tests the whole module.
void mymodule_test(error_log& log) {
  bool an_error_occurred {false};
  // Test code
  // Lets assume it sets the boolean to true.
  // This message does get appended.
  append_if("module failure", an_error_occurred);
}

// Now we collect all our unit tests and our module test together.
// Again, we define a function rather than a global object.
test_suite::pointer create_mymodule_test() {
  return create_test("mymodule", {
    create_test("top level test", mymodule_test),
    create_myclass_test(),
    create_my_other_class_test(),
    create_my_favorite_class_test(),
  });
}
```

Now we want to put all our module tests together in a file called 
`all_tests.cpp`. This will put together all our tests, run them and give
us a report.

```
#include "ttest.h"
#include <iostream>

using ttest::create_test;
using ttest::test_suite;

// Declare the module tests.
test_suite::pointer create_mymodule_test();
test_suite::pointer create_my_other_module_test();

int main() {
  auto big_test = create_test("my_program", {
    create_mymodule_test(),
    create_my_other_module_test(),
  });

  big_test->run_test();
  
  if (big_test->error_count() != 0) {
    big_test->report(std::cout);
  } else {
  std::cout << "No errors found\n";
  }

}
```

Lets assume that the only errors were the ones indicatecd in the code examples 
above.  Now if I didn't screw up my own example, the following output will be
generated. Please note that the order in which the messages are printed is
not entirely well defined. The messages from one sub module will not
be mixed with those from another sub module, but which sub module will be
tested first is not known.

```
my_program::mymodule::top level test::module failure
my_program::mymodule::myclass::testA::another failure message

```

In general, if you give reasonable names to your test_suite objects, it should 
be clear exactly which test function produced each error.
