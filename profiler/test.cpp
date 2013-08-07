#include <cppunit/TestAssert.h>
#include <cppunit/TestFixture.h>
#include <cppunit/extensions/HelperMacros.h>
#include <cppunit/extensions/TestFactoryRegistry.h>
#include <cppunit/ui/text/TestRunner.h>
#include <cstdlib>
#include <iostream>
#include <unistd.h>
#include "profiler.hpp"

void sleepMs(long ms) {
    usleep(ms * 1000);
}

/**
 * Exception handy for the tests
 */
class sleep_exception: public std::exception {
};

/**
 * Dummy class used for unit test
 */
class Klass
{
private:
    int time;

public:
    Klass(int sleepTime = 1): time(sleepTime), getXCalled(0), setZCalled(0) {}

    static unsigned staticMethodCalled;
    static void staticMethod(void) {
        ++staticMethodCalled;
        sleepMs(50);
    }

    unsigned getXCalled;
    int getX() {
        ++getXCalled;
        sleepMs(abs(time));
        if (time < 0)
            throw sleep_exception();
        return time;
    }

    unsigned setZCalled;
    void setZ(int z) {
        ++setZCalled;
        sleepMs(abs(z));
        if (z < 0)
            throw sleep_exception();
    }
};

unsigned Klass::staticMethodCalled = 0;

/**
 * Function to test wrapping of methods that return nothing
 */
static unsigned funcVoidCalled = 0;
void funcVoid(int sleepTime)
{
    ++funcVoidCalled;
    sleepMs(abs(sleepTime));
    if (sleepTime < 0)
        throw sleep_exception();

}

/**
 * Function to test wrappign of methods that return something
 */
static unsigned funcIntCalled = 0;
int funcInt(int sleepTime)
{
    ++funcIntCalled;
    sleepMs(abs(sleepTime));
    if (sleepTime < 0)
        throw sleep_exception();
    return sleepTime;
}

/**
 * Test suite for the profilers
 */
class TestProfiler: public CppUnit::TestFixture
{
public:

    void setUp() {
        funcVoidCalled = 0;
        funcIntCalled  = 0;
        Klass::staticMethodCalled = 0;
    }

    void testVoidFunction() {
        FunctionProfiler<void, int> fProf("funcVoid", funcVoid);
        for (int i = 0; i < 10; ++i)
            fProf(50);
        CPPUNIT_ASSERT_EQUAL(10ul, fProf.nCalls);
        CPPUNIT_ASSERT_EQUAL(0ul,  fProf.nExceptions);
        CPPUNIT_ASSERT(500 <= fProf.totalTime);
        CPPUNIT_ASSERT(550 >= fProf.totalTime);
        // Actually called
        CPPUNIT_ASSERT_EQUAL(10u, funcVoidCalled);
    }

    void testIntFunction() {
        FunctionProfiler<int, int> fProf("funcInt", funcInt);
        for (int i = 0; i < 20; ++i)
            CPPUNIT_ASSERT_EQUAL(20, fProf(20));

        CPPUNIT_ASSERT_EQUAL(20ul, fProf.nCalls);
        CPPUNIT_ASSERT_EQUAL(0ul,  fProf.nExceptions);
        CPPUNIT_ASSERT(400 <= fProf.totalTime);
        CPPUNIT_ASSERT(450 >= fProf.totalTime);
        // Actually called
        CPPUNIT_ASSERT_EQUAL(20u, funcIntCalled);
    }

    void testStaticMethod() {
        FunctionProfiler<void> fProf("Klass::staticMethod", Klass::staticMethod);
        for (int i = 0; i < 20; ++i)
            fProf();
        CPPUNIT_ASSERT_EQUAL(20ul, fProf.nCalls);
        CPPUNIT_ASSERT_EQUAL(0ul,  fProf.nExceptions);
        CPPUNIT_ASSERT(1000 <= fProf.totalTime);
        CPPUNIT_ASSERT(1050 >= fProf.totalTime);
        // Actually called
        CPPUNIT_ASSERT_EQUAL(20u, Klass::staticMethodCalled);
    }

    void testVoidMethod() {
        Klass instance(0);
        MethodProfiler<Klass, void, int> fProf("Klass::setZ", instance, &Klass::setZ);
        for (int i = 0; i < 15; ++i)
            fProf(15);
        CPPUNIT_ASSERT_EQUAL(15ul, fProf.nCalls);
        CPPUNIT_ASSERT_EQUAL(0ul,  fProf.nExceptions);
        CPPUNIT_ASSERT(225 <= fProf.totalTime);
        CPPUNIT_ASSERT(250 >= fProf.totalTime);
        // Actually called
        CPPUNIT_ASSERT_EQUAL(15u, instance.setZCalled);
    }

    void testIntMethod() {
        Klass instance(30);
        MethodProfiler<Klass, int> fProf("Klass::getX", instance, &Klass::getX);
        for (int i = 0; i < 30; ++i)
            CPPUNIT_ASSERT_EQUAL(30, fProf());
        CPPUNIT_ASSERT_EQUAL(30ul, fProf.nCalls);
        CPPUNIT_ASSERT_EQUAL(0ul,  fProf.nExceptions);
        CPPUNIT_ASSERT(900 <= fProf.totalTime);
        CPPUNIT_ASSERT(950 >= fProf.totalTime);
        // Actually called
        CPPUNIT_ASSERT_EQUAL(30u, instance.getXCalled);
    }

    void testVoidFunctionThrow() {
        FunctionProfiler<void, int> fProf("funcVoid", funcVoid);
        CPPUNIT_ASSERT_THROW(fProf(-10), sleep_exception);

        // Should have been count even though there was an exception
        CPPUNIT_ASSERT_EQUAL(1ul, fProf.nCalls);
        CPPUNIT_ASSERT_EQUAL(1ul,  fProf.nExceptions);
        CPPUNIT_ASSERT(10 <= fProf.totalTime);
        CPPUNIT_ASSERT(15 >= fProf.totalTime);
    }

    void testIntFunctionThrow() {
        FunctionProfiler<int, int> fProf("funcInt", funcInt);
        CPPUNIT_ASSERT_THROW(fProf(-10), sleep_exception);

        // Should have been count even though there was an exception
        CPPUNIT_ASSERT_EQUAL(1ul, fProf.nCalls);
        CPPUNIT_ASSERT_EQUAL(1ul,  fProf.nExceptions);
        CPPUNIT_ASSERT(10 <= fProf.totalTime);
        CPPUNIT_ASSERT(15 >= fProf.totalTime);
    }

    void testVoidMethodThrow() {
        Klass instance(0);
        MethodProfiler<Klass, void, int> fProf("Klass::setZ", instance, &Klass::setZ);
        for (int i = 0; i < 15; ++i)
            CPPUNIT_ASSERT_THROW(fProf(-15), sleep_exception);
        CPPUNIT_ASSERT_EQUAL(15ul, fProf.nCalls);
        CPPUNIT_ASSERT_EQUAL(15ul,  fProf.nExceptions);
        CPPUNIT_ASSERT(225 <= fProf.totalTime);
        CPPUNIT_ASSERT(250 >= fProf.totalTime);
        // Actually called
        CPPUNIT_ASSERT_EQUAL(15u, instance.setZCalled);
    }

    void testIntMethodThrow() {
        Klass instance(-10);
        MethodProfiler<Klass, int> fProf("Klass::getX", instance, &Klass::getX);
        for (int i = 0; i < 5; ++i)
            CPPUNIT_ASSERT_THROW(fProf(), sleep_exception);
        CPPUNIT_ASSERT_EQUAL(5ul, fProf.nCalls);
        CPPUNIT_ASSERT_EQUAL(5ul,  fProf.nExceptions);
        CPPUNIT_ASSERT(50 <= fProf.totalTime);
        CPPUNIT_ASSERT(60 >= fProf.totalTime);
        // Actually called
        CPPUNIT_ASSERT_EQUAL(5u, instance.getXCalled);
    }

    void testCombination() {
        FunctionProfiler<int, int> fProf("funcInt", funcInt);

        for (int i = 0; i < 5; ++i)
            CPPUNIT_ASSERT_THROW(fProf(-8), sleep_exception);
        for (int i = 0; i < 10; ++i)
            CPPUNIT_ASSERT_EQUAL(10, fProf(10));

        // Should have been count even though there was an exception
        CPPUNIT_ASSERT_EQUAL(15ul, fProf.nCalls);
        CPPUNIT_ASSERT_EQUAL( 5ul,  fProf.nExceptions);
        CPPUNIT_ASSERT(140 <= fProf.totalTime);
        CPPUNIT_ASSERT(160 >= fProf.totalTime);

        std::cerr << std::endl << fProf << std::endl;
    }

    void testReset() {
        FunctionProfiler<int, int> fProf("funcInt", funcInt);
        CPPUNIT_ASSERT_THROW(fProf(-8), sleep_exception);
        CPPUNIT_ASSERT_EQUAL(10, fProf(10));

        fProf.reset();

        CPPUNIT_ASSERT_EQUAL(0ul, fProf.nCalls);
        CPPUNIT_ASSERT_EQUAL(0ul, fProf.nExceptions);
        CPPUNIT_ASSERT_EQUAL(0.0, fProf.totalTime);
    }

    void testAggregator() {
        Klass instance(10);

        FunctionProfiler<int, int> profFInt("funcInt", funcInt);
        MethodProfiler<Klass, int> profMethod("Klass::getX", instance, &Klass::getX);

        ProfilerAggregator aggregator("testAggregator");

        aggregator.add(profFInt);
        aggregator.add(profMethod);

        CPPUNIT_ASSERT_THROW(profFInt(-10), sleep_exception);
        for (int i = 0; i < 5; ++i)
            CPPUNIT_ASSERT_EQUAL(10, profMethod());

        std::cerr << std::endl << aggregator << std::endl;

        aggregator.reset();

        std::cerr << std::endl << aggregator << std::endl;
    }

    CPPUNIT_TEST_SUITE(TestProfiler);
    CPPUNIT_TEST(testVoidFunction);
    CPPUNIT_TEST(testIntFunction);
    CPPUNIT_TEST(testStaticMethod);
    CPPUNIT_TEST(testVoidMethod);
    CPPUNIT_TEST(testIntMethod);
    CPPUNIT_TEST(testVoidFunctionThrow);
    CPPUNIT_TEST(testIntFunctionThrow);
    CPPUNIT_TEST(testVoidMethodThrow);
    CPPUNIT_TEST(testIntMethodThrow);
    CPPUNIT_TEST(testCombination);
    CPPUNIT_TEST(testReset);
    CPPUNIT_TEST(testAggregator);
    CPPUNIT_TEST_SUITE_END();
};

CPPUNIT_TEST_SUITE_REGISTRATION(TestProfiler);

/**
 * Entry point
 */
int main(int argc, char** argv)
{
    CppUnit::TextUi::TestRunner runner;
    CppUnit::TestFactoryRegistry &registry = CppUnit::TestFactoryRegistry::getRegistry();
    runner.addTest( registry.makeTest() );
    return runner.run("TestProfiler")?0:1;
}
