# Easy Test

This is a simple, lightweight test framework for C++ programs. I made it for
myself because I do not like writing tests and I do not like reading 
documentation. So the use of any other test framework has two strikes against
it right away. I made this as easy as possible to use because I knew that
if I ever forgot how to use it, then I would probably never use it again.
To that end, I made this is a header only library. There are no methods
with more than a few lines of code, so I have simply inlined every one. 
The interface has an easy learning curve. Of course that implies a certain lack
of flexibility as well. But for my needs, and maybe yours, all that is 
necessary is to run a series of test functions and then get a report listing
any errors that were found.

## Features

  1. SIMPLE INTERFACE: You need to know the following things:
    a. `#include "ttest.h"`. The extra t stands for "tree".
    b. Test functions have the signature `void(ttest::error_log&)`.
    c. `log.append("error message")` OR `log.append_if("msg", failure)`.
    d. The `create_test` function for collecting test functions together.
    e. all_my_tests->run_test();
    f. all_my_tests->report(std::cout);
    g. all_my_tests->error_count();

    I usually forget e, f, and g because I only write them once per project.
    So I have to look in my last project to remember the names. You can follow
    the examples here.

  2. HIERARCHICAL STRUCTURE: Tests are composed using the classic composite
    pattern. The actual class name is test_suite, but you won't need to 
    remember that. A test_suite object has the structure of a tree.
    A leaf in this tree is a test_suite object that holds a single test 
    function. A proper node is a test_suite object that itself holds a 
    collection of test_suite objects. The benefit of this structure is that
    it is as easy to define the test object for your entire library/program
    as it is for a single class method.

  3. GOOD REPORTING: Error messages are "scoped" so that you can easily tell
    exactly which test function produced the message without having to somehow
    encode that information in the actual error message.

  4. THREAD SAFETY: The error_log is thread safe. So your test functions can
    have concurrent threads writing to the log. Also, the code has been written
    in such as way as to enable the concurrent processing of test_functions
    once C++17 becomes the standard. I could have enabled that already of 
    course, but I didn't think it would be worth the trouble. C++17 will make
    it very easy because I will just have to add an execution policy to a
    single call of std::for_each.

## Requirements

  1. Your compiler must support C++14.

The following code example should illustrate everything you need to know to 
use this test framework.

UNFINISHED EXAMPLE

`example_tests.cpp`

```c++
#include "ttest.h"

using ttest::error_log;
using ttest::create_test;

void unit_testA(error_log& log) {
  // Set things up.

  // Here is one way to register an error.
  if (first_test_failed) {
    log.append("failure message");
  }

  // Here is another way.
  log.append_if("another failure message", second_test_failed);
}

void unit_testB(error_log& log) {
  // Another test that registers errors with the log.
}

// Now I want to t
void module_test_A(error_log&

```

  
  
  
