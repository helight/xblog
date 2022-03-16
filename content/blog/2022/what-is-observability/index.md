---
title: "可观察性:不是你想的那样"
date: 2022-03-08T08:45:20+08:00
tags: ["云原生"]
categories: ["云原生", "DevOps"]
banner: "img/banners/kubernetes.jpeg"
author: "helight"
authorlink: "http://helight.cn"
summary: ""
keywords: ["云原生", "DevOps"]
draft: true
---

本文译自 [可观察性:不是你想的那样](https://www.splunk.com/en_us/blog/devops/observability-it-s-not-what-you-think.html)。

译者：[helight](http://helight.cn/)。

原文地址：https://www.splunk.com/en_us/blog/devops/observability-it-s-not-what-you-think.html

## 前言
可观测性很多人估计会有一种疑惑：啥是可观测性，和我们之前的监控系统有啥区别，怎么老搞一些新名词出来，学不动了呀。。。。

所以这篇文章从理论的角度触发，为可观测性进行“强行”正名，我也想看看整这些名词的外国人是怎么想的。

## 可观察性是什么？
Observability is a mindset that enables you to answer any question about your entire business through collection and analysis of data. If you ask other folks, Observability is the dry control theory definition of “monitoring the internal state of a system by looking at its output,” or it’s the very technical definition of “metrics, traces, and logs.” While these are correct, Observability isn’t just one thing you implement, then proudly declare “now this system has Observability™.” Building Observability into your business lets you answer questions about your business.

## 有什么样的问题？
Of course, the basic “what happened in our app when this error count spiked up” questions can be answered with Observability tools, but that’s barely scratching the surface of what Observability actually is. What an Observability mindset lets you do is to figure out why the error count spiked up. If you’re intimately familiar with your app and all of its dependencies, then perhaps you can get this insight from a monitoring system, but as modern apps become increasingly more complex, the ability to maintain the state of them in your head becomes more and more challenging. Business demands, feature launches, A/B tests, refactoring into microservices… things like this all combine to create ever-increasing entropy, so knowing everything about your system without help gets more difficult by the day. 

Observability also lets you ask how (or if!) the errors actually impacted the user experience. You can look at RUM data, purchase volume, general business metrics, marketing campaigns, customer support tickets, social media sentiment, the list goes on and on – this data takes an Observability system from something only a few people use to something the entire company can get insight from. This data lets you answer not just “what,” but “why” and “how.” A true Observability suite lets you answer all of these:

“What made this break?”
“How effective was this ad?”
“Did this new front-end design drive purchases?”
“Did this service outage make our users angry?”

## 你为什么应该关心可观测性？
Integrating this type of data into your system lets you discover that a marketing campaign you sent to your best customers had a typo in the call to action URL that is sending customers to a 404 page. Without fully integrating data into your Observability solution, sure, you can see the 4xx rate increased. However, you can’t figure out why the 4xx rate went up, only that the 4xx rate went up. Imagine how much faster you could resolve an issue if in addition to “fe-server 4xx above threshold” you also saw an event showing that “marketing campaign whales-winback started” happened at the same time. You’d know not only what was wrong, but you’d have a good guess at what caused it, and you’d have a good springboard to investigate the revenue impact, or negative goodwill, that this error caused you.

## 为什么说这不是监控？
As I said, monitoring tells you something is wrong, but it doesn’t tell you why it’s wrong. Monitoring setups also can only monitor things you’ve already thought could be problematic (your ‘known knowns’.) If you didn’t think to instrument the component in question in advance, you can’t monitor it. What’s worse, if you then have a problem there and decide to add monitoring to it, you still don’t have the historical data about how the component performed. Also, monitoring requires special attention before you even know what could go wrong – you have to specifically instrument specific things and set up specific alerts about them. This takes time and is prone to errors.

Also, no matter how well-instrumented your monitoring solution is, it still doesn’t let you explore your business. Looking into ‘unknown unknowns’ isn’t possible with a classic monitoring system, because the data simply doesn’t exist for you to evaluate. Adding in business metrics is generally not supported or poorly supported in traditional monitoring. Real-user data is almost never included in monitoring systems, which is absurd, because the entire point of what we do in web applications is delivering user experience!

## 可观测性“三大支柱”如何协同工作？
Metrics, traces, and logs are the ‘three pillars’ of Observability, and they are necessary but not sufficient to really understand what Observability is and to gain insight into your applications and business. Metrics can be used to tell you what’s wrong. Traces tell you how it’s wrong – what specific calls aren’t working, for example. Logs tell you why it’s wrong, letting you dive into a particular metric/trace to figure out why it’s behaving in the way you see. Collecting this data is the start to an Observability mindset, but it’s only the start.

## 为什么你需要每一块数据？
A big problem with Observability from the naive point of view is that there’s simply too much data to collect and retain it all. “You can’t realistically store the amount of output generated by a modern service in one place,” goes the refrain. The solution to this that most vendors propose is what they call sampling, but what I like to call “throwing data away.” The data that gets thrown away might be your most critical customer’s transaction. It might be the one particular use case that causes some bizarre bug that crashes your database server. What’s worse is that a lot of vendors will advertise this as a feature to ‘save you money.’ I'll go into more detail about the hidden costs of sampling in a separate blog post.

In a classic control-theory world where you had a bunch of gauges monitoring critical infrastructure, would you throw out 70% of the observations because “30% should be good enough?” Of course you wouldn’t, yet this is what many vendors suggest you simply must do with Observability due to the nature of the data in question. It isn't true. Mature platforms can handle all the data about your business without throwing any away.

## 可观察性不是一种实际操作，而是一种观念
While this article has discussed some implementation details about Observability, what Observability really is isn’t “collect and store metrics, traces, and logs.” It’s a mindset of “what data should we collect that might be useful in figuring out any question we want to know about our business.” Observability is not merely about application performance monitoring or infrastructure monitoring (though those are parts of it). It’s about understanding the need to ingest everything. Real-user experience metrics. Marketing campaigns. Seasonal changes to traffic. Sick days taken by your warehouse team. 

Observability is a mindset that necessitates a single source of truth for data about your business and your applications that everyone (developers, ops, product, the C-suite, etc.) uses. There’s millions of points of data that make up your business, and Observability is about capturing all that data in one system and then using the data to answer questions beyond just the technical app(s) your business runs.

To fully leverage Observability, you need a purpose-built streaming architecture that can arbitrarily scale and that lets you receive constant feedback on how your changes impact your users and your business. You need a system that bundles together many tools into one common source of truth and that provides insights from those tools.


<center>
看完本文有收获？请分享给更多人

关注「黑光技术」，关注大数据+微服务

![](/img/qrcode_helight_tech.jpg)

</center>