add_executable(exokomodo.drakon.tests.stub stub.cpp)
set_property(TARGET exokomodo.drakon.tests.stub PROPERTY CXX_STANDARD 20)
add_test(NAME exokomodo.drakon.tests.stub COMMAND exokomodo.drakon.tests.stub)
