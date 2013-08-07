#ifndef _PROFILER_H_
#define _PROFILER_H_

#include <ctime>
#include <list>
#include <string>
#include <sstream>

/**
 * Get the current monotonic time in milliseconds
 */
inline double getMilliseconds() {
    struct timespec ts;
    clock_gettime(CLOCK_MONOTONIC, &ts);
    return ts.tv_nsec / 1000000 + ts.tv_sec * 1000;
}

/**
 * Holds the data. Done like this so this base class
 * doesn't use templates
 */
class BaseProfiler {
public:
    std::string   name;
    double        totalTime; // In milliseconds!
    long unsigned nCalls;
    long unsigned nExceptions;

    BaseProfiler(const std::string& name):
        name(name), totalTime(0), nCalls(0), nExceptions(0) {
    }

    virtual ~BaseProfiler() {
    }

    void reset() {
        totalTime   = 0;
        nCalls      = 0;
        nExceptions = 0;
    }
};


std::ostream& operator << (std::ostream& out, const BaseProfiler& prof) {
    double average = 0;
    if (prof.nCalls)
        average = prof.totalTime / prof.nCalls;
    out << '`' << prof.name << "` called " << prof.nCalls << " times, "
        << average << " ms average, "
        << "has thrown " << prof.nExceptions << " exceptions";
    return out;
}

/**
 * Base class for specific profilers.
 * Profilers are callable, and hold a reference to the
 * profiled underlying callable
 */
template <class RTYPE, typename... ARGS>
class CallableProfiler: public BaseProfiler {
private:
    double start;

protected:
    void profileStart() {
        start = getMilliseconds();
    }

    void profileEnd() {
        double end = getMilliseconds();
        totalTime += end - start;
        ++nCalls;
    }

    void profileEndWithException() {
        double end = getMilliseconds();
        totalTime += end - start;
        ++nCalls;
        ++nExceptions;
    }

public:

    CallableProfiler(const std::string& name):
        BaseProfiler(name), start(0) {
    }

    virtual ~CallableProfiler() {};

    virtual RTYPE operator () (ARGS...) = 0;
};

/**
 * This profilers holds a pointer to a regular function
 */
template <class RTYPE, typename... ARGS>
class FunctionProfiler: public CallableProfiler<RTYPE, ARGS...> {
public:
    typedef RTYPE(*FPtr)(ARGS...);
    FPtr fptr;

    FunctionProfiler(const std::string& name, FPtr func):
        CallableProfiler<RTYPE, ARGS...>(name), fptr(func) {
    }

    RTYPE operator () (ARGS... args) {
        this->profileStart();
        try {
            RTYPE r = fptr(args...);
            this->profileEnd();
            return r;
        }
        catch (...) {
            this->profileEndWithException();
            throw;
        }
    }
};

/**
 * Specialization of Functionprofiler for functions that return null
 */
template <typename... ARGS>
class FunctionProfiler<void, ARGS...>: public CallableProfiler<void, ARGS...> {
public:
    typedef void(*FPtr)(ARGS...);
    FPtr fptr;

    FunctionProfiler(const std::string& name, FPtr func):
        CallableProfiler<void, ARGS...>(name), fptr(func) {
    }

    void operator () (ARGS... args) {
        this->profileStart();
        try {
            fptr(args...);
            this->profileEnd();
        }
        catch (...) {
            this->profileEndWithException();
            throw;
        }
    }
};

/**
 * Holds a method bound to an instance
 */
template <class KLASS, class RTYPE, typename... ARGS>
class MethodProfiler: public CallableProfiler<RTYPE, ARGS...> {
public:
    typedef RTYPE(KLASS::*FPtr)(ARGS...);

    KLASS& instance;
    FPtr   fptr;

    MethodProfiler(const std::string& name, KLASS& instance, FPtr f):
        CallableProfiler<RTYPE, ARGS...>(name), instance(instance), fptr(f){
    }

    RTYPE operator () (ARGS... args) {
        this->profileStart();
        try {
            RTYPE r = (instance.*fptr)(args...);
            this->profileEnd();
            return r;
        }
        catch (...) {
            this->profileEndWithException();
            throw;
        }
    }
};

/**
 * Holds a method bound to an instance.
 * Specialization for void methods
 */
template <class KLASS, typename... ARGS>
class MethodProfiler<KLASS, void, ARGS...>: public CallableProfiler<void, ARGS...> {
public:
    typedef void(KLASS::*FPtr)(ARGS...);

    KLASS& instance;
    FPtr   fptr;

    MethodProfiler(const std::string& name, KLASS& instance, FPtr f):
        CallableProfiler<void, ARGS...>(name), instance(instance), fptr(f){
    }

    void operator () (ARGS... args) {
        this->profileStart();
        try {
            (instance.*fptr)(args...);
            this->profileEnd();
        }
        catch (...) {
            this->profileEndWithException();
            throw;
        }
    }
};

/**
 * Aggregates several profilers just for convenience
 */
class ProfilerAggregator {
protected:
    std::list<BaseProfiler*> profilers;

    friend std::ostream& operator << (std::ostream&, const ProfilerAggregator&);

public:
    std::string label;

    ProfilerAggregator(const std::string& label): label(label) {
    }

    void add(BaseProfiler& prof) {
        profilers.push_back(&prof);
    }

    void reset() {
        std::list<BaseProfiler*>::iterator i;
        for (i = profilers.begin(); i != profilers.end(); ++i)
            (*i)->reset();
    }
};

std::ostream& operator << (std::ostream& out, const ProfilerAggregator& profAggr) {
    std::list<BaseProfiler*>::const_iterator i;

    out << "[" << profAggr.label << "]" << std::endl;

    for (i = profAggr.profilers.begin(); i != profAggr.profilers.end(); ++i) {
        out << '\t' << **i << std::endl;
    }
    return out;
}

#endif // _PROFILER_H_
