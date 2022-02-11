# User-level thread library
## Implementation
### Design
High level description of design choice for the three files.
#### queue.c
For this part we decide to use a single linked-list for our queue 
structure. Although it is required that all of our operations of the queue
(apart from iterate and delete operations) to be O(1), we think that a 
single linked list implementation suffices, as long as we keep both head
and tail pointers. A head pointer always points to the head of the queue
and a tail pointer always points to the end of the queue. In this way,
by proper maintenance of the head and tail pointers, queue_enqueue,
queue_dequeue, queue_create, queue_destroy all take O(1) time.
include any loops in these operations as well. If we did include loops in 
these operations, it would most likely lead to a time complexity greater than 
O(1).
#### uthread.c
For this part, it is important to decide what should be included in the 
TCB struct, since these fields in the TCB struct are highly correlated to 
the implementation of each uthread functions. We decide that it should 
contain uthread_t tid, State state, uthread_ctx_t* context, void* sp,
int retval, TCB_t joined_by, TCB_t join, where TCB_t is a pointer to TCB
struct. tid is an unsigned short globally and uniquely identifying the thread. 
state denotes the current state of the thread, where possible value for the 
state is UNINITIALIZED, READY, RUNNING, BLOCKED, ZOMBIE. UNINITIALIZED is the 
state just created but not ready to run yet. READY is the state to be scheduled to 
run. RUNNING is the state currently holding the CPU. BLOCKED is the state
being blocked. ZOMBIE is the state exited but its status not collected yet.
context stores the execution context of the thread. sp stores the top of the 
stack of the thread. retval is used to store the exit status of the thread,
so that it can later be collected. joined_by denotes the other thread that 
joins the current thread. join denotes the other thread joined by the current
thread. Note that at any moment, a thread is joined by at most one thread and 
join at most one thread. It follows that there must be a global uthread_t 
to keep track of the next available uthread_t. In addition, there should be 
three queues, ready_queue, blocked_queue, zombie_queue to store ready threads,
blocked threads and zombie threads respectively. There should also be a global
TCB pointer denoting the current active thread, so that when implementing 
uthread_yield, uthread_join etc, we know which thread to switch from.
#### preempt.c
sigset_t vt_alarm;
struct itimerval old_timerval;
struct itimerval new_timerval;
struct sigaction old_action;
struct sigaction new_action;
void sigvalrm_handler();
int preemption_set = 0;
For this part, we decide that there should be an alarm to be enabled or
disabled. Besides, there should be two struct sigaction, one for the 
project requirement and one for restoring the old sigaction in preempt_stop().
In addition, there should be two struct itimerval, one for setting the
required timer interval and one for restoring the original timer interval 
int preempt_stop(). There should also be a flag to remember whether 
preempt mode is set or not, so that in preempt_stop(), if preempty mode 
is set, we restore the original sigaction and itimerval correspondingly.

preempt_disable() and preempt_enable() are called in necessary place to
guard against global variable and date structures that would possibly
cause read write conflict, thus consequently lead to race condition
and inconsistent consequence.
### Implementation
Simply code the above ideas in C.
#### queue.c
##### queue_create
Allocate storage for the queue. Set head and tail to point to NULL.
Set size of queue to 0
##### queue_destroy
If queue is empty, release the memory acquired by queue.
##### queue_enqueue
Create a new node to hold the data. Set the new node as the next of tail.
Set tail to point to the new node. Increment size of queue by 1.
##### queue_dequeue
Set head to point to the next of head. Free of the memory head previously
pointing to. Decrement the size of queue by 1.
##### queue_delete
Traverse the queue, start from the head of the queue, use two pointers, 
one points to the previous node of the current node, one points to the 
current node. If date is found in the current node, set the next of the 
previous pointer to the next of the current pointer, and free memory 
correspondingly. If freed successfully, decrement the queue size by 1.
##### queue_iterate
Traverse the queue, start from the head of the queue, use two pointers,
one points to the next node of the current node, one points to the
current node. Apply the func to the date in the current node. If func return
1 and the passed data is not NULL, store the data in current node to the data
pointer passed in, then return. Otherwise, set the current node to the node
pointed to by the next node, and then set the next node to point to the next
node of the current node. Repeat this until the end of the queue. The reason
of using two pointer to traverse the queue is to be resistant to data items 
being deleted as part of the iteration in func. Because we store the next 
node of the current node in advance, even if in func, the current node is 
deleted, we can still move to the next node of the current node by simply 
assigning the prestored next node to the current node.
##### queue_length
we just return the size of the queue or -1 of the queue is NULL.
#### uthread.c
##### uthread_start
Set global uthread_t to 0. Initialize ready_queue, blocked_queue and zombie
queue. Allocate TCB for the current thread and set its state to be RUNNING.
If preempty is set, call preempt_start().
##### uthread_stop
Should check if the current_thread's id is the same as the main thread's id
Besides, should check whether ready_queue and blocked_queue is empty.
If zombie queue is not empty, we can delete all the nodes in the zomibe 
queue. 
##### uthread_create
Allocate space for the TCB, initialize the context with the func argument.
Then set its state to be READY and put it in the ready queue.
##### uthread_self
return the tid of the current thread.
##### uthread_yield
Add the current thread to the ready queue. Pick a thread from the ready 
queue, set it as the current thread and set its state to be RUNNING.
Do a context switch by calling uthread_ctx_switch.
##### uthread_exit
Put the current thread to the zombie queue, set its state to be ZOMBIE. If 
the current thead is joined by another thread and if the thread is in 
blocked state. Delete the join thread form the blocked queue, set its 
state to READY, put it to the ready queue. Call uthread_yield to let
other ready threads to be scheduled.
##### uthread_join
Have to check if tid = 0 and tid = current thread's tid and if the tid 
can be found in the three queues and if the thread has already been joined.
If the thread is zombie state, then its exit status can be collected 
immediately. Otherwise, set current thread to be BLOCKED and put it to 
blocked queue. Set join and joined_by accordingly. Wait for wake up.
In both case, when the joined is in zombie state, collect its exit status, 
delete it from the zombie queue and free its TCB storage.
### preempty.c
#### preempt_start
Set SIGVTALRM signal handler and timer for ITIMER_VIRTUAL.
Store the old sigaction and itimerval so that they can be restored later.
Set the preempty flat to be 1 indicating that preemption mode is set.
#### preempt_stop
if preempty flat is set, restore the original sigaction and timer.
#### preempt_enable
Use sigprocmask to SIG_UNBLOCK the SIGVTALRM.
#### preempt_disable
Use sigprocmask to SIG_BLOCK the SIGVTALRM.

### Testing
It is easy to make mistakes in implementing queue_iterate(), especially
it is required to be resistant to data items being deleted as part of 
the iteration in func. We have pay special attention to that in 
test_queue_iterate_stop_premature() and test_iterator_complex() in queue_tester.c.
For uthread.c and preempt.c, it is best illustrated in test_preempt.c and
test_preempt_alternating.c.
#### queue.c
Run ./queue_tester.x
Should see PASS for all the cases.
#### uthread.c
Run ./uthread_hello.x
Should see Hello world!

./uthread_yield.x
Shoul see:
thread1
thread2
thread3

#### preempt.c
Run ./test_preempt.x
Should see:
in thread2: tid= 2
return from join in main
Warning: there are still user threads in ready queue or blocked queue

Run ./test_preempt_alternating.x
Should see "in thread1: tid= 1" and "in thread2: tid= 2" alternating 
forever. Each message would occupy 30-80 thousand lines consecutively 
before switch to the other message depending on the machine and the 
scheduler. This can be verified by running 
./test_preempt_alternating.x > log.txt, then find the patter thread1 and
thread2 in log.txt

### Outside Sources
For reference:
* [Linux man pages](https://linux.die.net/man/)
* https://www.kernel.org/doc/html/latest/process/coding-style.html
* http://tldp.org/HOWTO/Program-Library-HOWTO/static-libraries.html
* https://www.gnu.org/software/libc/manual/html_mono/libc.html#System-V-contexts
* https://www.gnu.org/software/libc/manual/html_mono/libc.html#Signal-Actions
* https://www.gnu.org/software/libc/manual/html_mono/libc.html#Setting-an-Alarm
* https://www.gnu.org/software/libc/manual/html_mono/libc.html#Blocking-Signals