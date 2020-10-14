

I don't think it's sufficient to just measure how long a piece of code takes to execute. Your environment is a constantly changing thing, so you have to take a statistical approach to measuring execution time.

Essentially you need to take N measurements, discard outliers, and calculate your average, median and standard deviation running time, with an uncertainty measurement.

Here's a good blog explaining why and how to do this (with code):

http://blogs.perl.org/users/steffen_mueller/2010/09/your-benchmarks-suck.html

https://stackoverflow.com/questions/7456146/is-there-a-better-way-to-benchmark-a-c-program-than-timing

