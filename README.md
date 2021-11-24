# lock-implementations
Implements and tests several lock implementations including TAS, TTAS with backoff, and and array lock. The locks are test by incrementing a shared counter and their performance is compared to the standard pthreads mutex lock.
