#include <pthread.h>
#include <iostream>

int main() {
    pthread_attr_t attrs;
    pthread_getattr_default_np(&attrs);

    int scope;
    pthread_attr_getscope(&attrs, &scope);
    std::cout << "Scope: " << (
            (scope == PTHREAD_SCOPE_SYSTEM) ? "SYSTEM" :
            (scope == PTHREAD_SCOPE_PROCESS) ? "PROCESS" :
            "unknown") << std::endl;

    int detachstate;
    pthread_attr_getdetachstate(&attrs, &detachstate);
    std::cout << "Detach state: " << (
            (detachstate == PTHREAD_CREATE_DETACHED) ? "DETACHED" :
            (detachstate == PTHREAD_CREATE_JOINABLE) ? "JOINABLE" :
            "unknown") << std::endl;

    void *stackptr;
    size_t stacksize;
    pthread_attr_getstack(&attrs, &stackptr, &stacksize);
    std::cout << "Stack address: " << stackptr << std::endl;
    std::cout << "Stack size: " << stacksize << std::endl;

    int inheritsched;
    pthread_attr_getinheritsched(&attrs, &inheritsched);
    std::cout << "Inherit scheduler: " << (
                   (inheritsched == PTHREAD_INHERIT_SCHED) ? "INHERIT" :
                   (inheritsched == PTHREAD_EXPLICIT_SCHED) ? "EXPLICIT" :
                   "unknown") << std::endl;
    int policy;
    pthread_attr_getschedpolicy(&attrs, &policy);
    std::cout << "Scheduling policy: " << (
            (policy == SCHED_FIFO) ? "SCHED_FIFO" :
            (policy == SCHED_RR) ? "SCHED_RR" :
            (policy == SCHED_OTHER) ? "SCHED_OTHER" :
            "unknown") << std::endl;
}
