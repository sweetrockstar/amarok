// Author: Max Howell (C) Copyright 2004
// Copyright: See COPYING file that comes with this distribution
//

#ifndef THREADWEAVER_H
#define THREADWEAVER_H

#include <qevent.h>   //baseclass
#include <qguardedptr.h>
#include <qmap.h>
#include <qobject.h>
#include <qthread.h>
#include <qvaluelist.h>


//not all these are always generated, but the idea is just to prevent it
#define DISABLE_GENERATED_MEMBER_FUNCTIONS( T ) \
    T(); \
    T( const T& ); \
    T &operator=( const T& ); \
    bool operator==( const T& ) const;


/**
* @class ThreadWeaver
* @author Max Howell <max.howell@methylblue.com>
* @short ThreadWeaver is designed to encourage you to use threads and to make their use easy.
*
* You create Jobs on the heap and ThreadWeaver allows you to easily queue them,
* abort them, ensure only one runs at once, ensure that bad data is never acted
* on and even cleans up for you should the class that wants the Job's results
* get deleted while a thread is running.
*
* You also will (soon) get thread-safe error handling and thread-safe progress
* reporting.
*
* This is a typical use:
*
    class MyJob : public ThreadWeaver::Job
    {
    public:
        MyJob( QObject *dependent ) : Job( dependent, "MyJob" ) {}

        virtual bool doJob() {
            //do some work in thread...

            return success;
        }

        virtual void completeJob {
            //do completion work in the GUI thread...
        }
    };

    SomeClass::someFunction()
    {
        ThreadWeaver::instance()->queueJob( new MyJob( this ) );
    }

*
* That's it! The queue is fifo, the ThreadWeaver takes ownership of the Job,
* and the Weaver calls Job::completeJob() on completion which you reimplement
* to do whatever you need done.
*
* @see ThreadWeaver::Job
*/

class ThreadWeaver : public QObject
{
    Q_OBJECT

public:
    class Job;
    typedef QValueList<Job*> JobList;

    enum EventType { JobEvent = 2000, JobStartedEvent = JobEvent, JobFinishedEvent, DeleteThreadEvent };

    static ThreadWeaver *instance();

    /**
     * If the ThreadWeaver is already handling a job of this type then the job
     * will be queued, otherwise the job will be processed immediately. Allocate
     * the job on the heap, and ThreadWeaver will delete it for you.
     *
     * This is not thread-safe - only call it from the GUI-thread!
     *
     * @return number of jobs in the queue after the call
     * @see ThreadWeaver::Job
     */
    int queueJob( Job* );

    /**
     * Queue multiple jobs simultaneously, you should use this to avoid the race
     * condition where the first job finishes before you can queue the next one.
     * This isn't a fatal condition, but it does cause wasteful thread deletion
     * and re-creation. The only valid usage, is when the jobs are the same type!
     *
     * This is not thread-safe - only call it from the GUI-thread!
     *
     * @return number of jobs in the queue after the call
     */
    int queueJobs( const JobList& );

    /**
     * If there are other jobs of the same type running, they will be aborted,
     * then this one will be started afterwards. Aborted jobs will not have
     * completeJob() called for them.
     *
     * This is not thread-safe - only call it from the GUI-thread!
     */
    void onlyOneJob( Job* );

    /**
     * All the named jobs will be halted and deleted. You cannot use any data
     * from the jobs reliably after this point. Job::completeJob() will not be
     * called for any of these jobs.
     *
     * This is not thread-safe - only call it from the GUI-thread!
     *
     * @return how many jobs were aborted, or -1 if no thread was found
     */
    int abortAllJobsNamed( const QCString &name );

private slots:
    void dependentAboutToBeDestroyed();

private:
    ThreadWeaver();
   ~ThreadWeaver();

    virtual void customEvent( QCustomEvent* );


    /**
     * Class Thread
     */
    class Thread : public QThread
    {
    public:
        Thread( const char* );

        virtual void run();

        void runJob( Job *job );

        JobList pendingJobs;
        Job *runningJob;

        char const * const name;

        void abortAllJobs();

        int jobCount() const { return pendingJobs.count() + runningJob ? 1 : 0; }

        void msleep( int ms ) { QThread::msleep( ms ); } //we need to make this public for class Job

    protected:
        DISABLE_GENERATED_MEMBER_FUNCTIONS( Thread );

    private:
        friend void ThreadWeaver::customEvent( QCustomEvent* );

        ~Thread();
    };


    /// safe disposal for threads that may not have finished
    void dispose( Thread* );

    /// returns the thread that handles Jobs that use @name
    inline Thread *findThread( const QCString &name );

    typedef QValueList<Thread*> ThreadList;

    ThreadList m_threads;

public:
    /**
     * @class Job
     * @short A small class for doing work in a background thread
     *
     * Derive a job, do the work in doJob(), do GUI-safe operations in
     * completeJob(). If you return false from doJob() completeJob() won't be
     * called. Name your Job well as like-named Jobs are queued together.
     *
     * Be sensible and pass data members to the Job, rather than operate on
     * volatile data members in the GUI-thread.
     */

    class Job : public QCustomEvent
    {
        friend class ThreadWeaver;
        friend class ThreadWeaver::Thread;

    public:
        /**
         * Like-named jobs are queued and run FIFO. Always allocate Jobs on the
         * heap, ThreadWeaver will take ownership of the memory.
         */
        Job( const char *name );
       ~Job();

        const char *name() const { return m_name; }

        /**
         * If this returns true then in the worst case the entire amaroK UI is
         * frozen waiting for your Job to abort! You should check for this
         * often, but not so often that your code's readability suffers as a
         * result.
         *
         * Aborted jobs will not have completeJob() called for them, even if
         * they return true from doJob()
         */
        bool isAborted() const { return m_aborted; }

        /**
         * Calls QThread::msleep( int )
         */
        void msleep( int ms ) { m_thread->msleep( ms ); }

    protected:
        /**
         * Executed inside the thread, this should be reimplemented to do the
         * job's work. Be thread-safe! Don't interact with the GUI-thread.
         *
         * @return true if you want completeJob() to be called from the GUI
         * thread
         */
        virtual bool doJob() = 0;

        /**
         * This is executed in the GUI thread if doJob() returns true;
         */
        virtual void completeJob() = 0;

    private:
        char const * const m_name;
        bool m_aborted;
        Thread *m_thread;

    protected:
        DISABLE_GENERATED_MEMBER_FUNCTIONS( Job )
    };


    /**
     * @class DependentJob
     * @short A Job that depends on the existence of a QObject
     *
     * This Job type is dependent on a QObject instance, if that instance is
     * deleted, this Job will be aborted and safely deleted.
     *
     * completeJob() is reimplemented to send a JobFinishedEvent to the
     * dependent. Of course, you can still reimplement it yourself.
     *
     * The dependent is a QGuardedPtr, so you can reference the pointer returned
     * from dependent() safely provided you always test for 0 first. However
     * safest of all is to not rely on that pointer at all! Pass required
     * data-members with the job, only operate on the dependent in
     * completeJob(). completeJob() will not be called if the dependent no
     * longer exists
     *
     * It is only safe to have one dependent, if you depend on multiple objects
     * that might get deleted while you are running you should instead try to
     * make the multiple objects children of one QObject and depend on the
     * top-most parent or best of all would be to make copies of the data you
     * need instead of being dependent.
     */

    class DependentJob : public Job
    {
    public:
        DependentJob( QObject *dependent, const char *name );

        virtual void completeJob();

        QObject *dependent() { return m_dependent; }

    private:
        const QGuardedPtr<QObject> m_dependent;

    protected:
        DISABLE_GENERATED_MEMBER_FUNCTIONS( DependentJob );
    };

private:
    friend DependentJob::DependentJob( QObject*, const char* );
    friend Thread::~Thread();

    void registerDependent( QObject*, const char* );

protected:
    ThreadWeaver( const ThreadWeaver& );
    ThreadWeaver &operator=( const ThreadWeaver& );
};

inline ThreadWeaver*
ThreadWeaver::instance()
{
    static ThreadWeaver instance;

    return &instance;
}

#endif
