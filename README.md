# Asynchronous Userspace Non-Preemptive Fixed Priority Scheduled Work Pool

The name is definitely a mouthfull but this was a project I did over the weekend for myself to help with another project I'm working on, it's a generic/templated work/thread pool implementation that is scheduled in a fixed priority manner in (semi) bounded time due to using a red-black tree.

The scheduler is meant for VERY loose task scheduling in userspace as it is non-preemptive and a userspace application so it doesn't have access to high resolution timing features that requires hardware access.
