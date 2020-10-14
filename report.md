
# how to bench
https://stackoverflow.com/questions/7456146/is-there-a-better-way-to-benchmark-a-c-program-than-timing
I don't think it's sufficient to just measure how long a piece of code takes to execute. Your environment is a constantly changing thing, so you have to take a statistical approach to measuring execution time.

Essentially you need to take N measurements, discard outliers, and calculate your average, median and standard deviation running time, with an uncertainty measurement.

Here's a good blog explaining why and how to do this (with code):

http://blogs.perl.org/users/steffen_mueller/2010/09/your-benchmarks-suck.html

# what is cdf
Percentile means people behind you.

If you are in the top 10%, your are in the 90th percentile.

If you are in the top 1%, you are in the 99th percentile.

But shouldn’t 100th percentile be the best? No, that’s because you can’t be in the top 0%. Being in the top 0% means all people including yourself scored worse than you. But you can’t score worse than yourself, can you?

So 99.9th percentile is the best, followed by 99.8th percentile. This goes all the way to 0th percentile. It is possible for no one to score worse than you, but it is impossible for everyone (that includes you) to score worse than you. Hence, 0th percentile is possible while 100th percentile is impossible.

