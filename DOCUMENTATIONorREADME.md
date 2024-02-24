# Parallel-Processing-Assignment-2

This is where the documentation goes, right? So, both of the programs simulating the answer to the problems are in the one file, Assignment2.cpp. Just compile and run it, and it will
prompt you for N for each question. 

For the first problem, I definitely tangled with this for a while. Ultimately I realized that the total number of guests was a piece of information the guests would obviously
have access to, and from there it became clearer. Since the threads are "declared" sequentially I went ahead and just said the first one would always be the counter of the cupcakes.
As in, he counts them whenever he calls for a replacement cupcake. More info on that specific solution is found in the code comments. Initially I had it the otehr way around,
where he would eat the cupcakes and count that, but for whatever reason the cupcake "states" make more sense in my head if the other guests are the ones leaving an empty plate.

For the second problem, I went with the third solution, the queue. From a code angle, I couldn't really discern a difference between answers one and two. Guests were never allowed to
be in the showroom with another guest at the same time in the first place, so it just didn't make sense how either was particularily different from an implementation perspective. That
implementation, by the way, would just have the threads automatically place themselves behind a mutex lock for the showroom. The thing is, at least for windows, it seems thread ownership
of a lock is handled via a sort of first-in-first-out methodology, in other words, it's *almost* a queue. All in all the third solution just makes the most sense when you consider
the actual scenario at hand. It's more polite if all these guests just line up, and this guarantees that everyone that wants to see it can see it. The trouble I ran into was getting
this to actually model itself. A queue is pretty simple structure, so in early versions, you couldn't really tell that any sort of parallelism was happening. It was all pretty linear.
The threads declared first would line up first, so they would just go in one after another. So first I added in a random value to determine if a guest would line up at all in the first
place, and then other random value after viewing the vase to determine if they wanted to line up a second time. Then I added in a sleep() function inside the showroom. In most real "code"
situations where you'd have this sort of thread structure, the multithreading would come in handy as there's some nontrivial amount of "work" done inside the body (in this case, the showroom.)
However, no real code happens inside the showroom, so in order to have things actually working concurrently, I just send the currently-occupying thread to sleep for a bit so the other threads have
time to actually do something in the background. It's for this reason that I don't recommend using large N values for the second problem. It'll work fine in the end, it's just going to
intentionally take a while.
