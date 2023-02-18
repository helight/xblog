---
title: "平台工程: 入门所需的所有知识"
date: 2023-02-18T08:45:20+08:00
tags: ["DevOps", "SRE"]
categories: ["DevOps", "SRE"]
banner: "img/banners/kubernetes.jpeg"
author: "helight"
authorlink: "http://helight.cn"
summary: ""
keywords: ["DevOps", "SRE"]
draft: true
---

# Platform Engineering 101: all you need to know to get started


Digital platforms are shaping a **new way to provide services and engage their users**. Thanks to them, it is possible to offer an omnichannel experience to customers, reaching all the touchpoints by which they interact with the company. Thus, the organization can take advantage of these opportunities to create new business models.
数字平台正在形成一种提供服务和吸引用户的新方式。多亏了他们，才有可能为客户提供全渠道的体验，达到他们与公司互动的所有接触点。因此，组织可以利用这些机会来创建新的业务模型。

Given their widespread use, in this article we want to provide a guide to understanding the fundamentals of the entire ecosystem that has been created and developed around the whole phenomenon of digital platforms. In particular, we will explain the **basic concepts** that you need to know to start working with cloud platforms, providing a glossary of the most important terms.
鉴于它们的广泛使用，在本文中，我们希望提供一个指南，以理解围绕数字平台的整个现象而创建和发展的整个生态系统的基本原理。特别地，我们将解释开始使用云平台时需要了解的**基本概念**，并提供最重要的术语表。

## Digital Platform: understanding the reasons for success

Certainly, the first concept to learn about is the **digital platform**, the tool behind the digital transformation. The digital platform **bridges the digital gap** between external channels, through which users interact with the organization, and the organization's internal systems (e.g., CRM, e‑commerce, management systems, etc.). Moreover, the platform helps **offload the systems**, increases the scalability and flexibility of the services offered, and decreases the time to market.

A well‑structured platform is usually developed using modern cloud‑native technologies, and is divided into three levels:

-   A data management layer: data is decoupled from the organization's systems, aggregated in a single view, and finally made available in near [real‑time](https://blog.mia-platform.eu/en/fast-data-evolve-your-users-experience-with-real-time-information).
-   A layer dedicated to business logic: here a [microservice](https://blog.mia-platform.eu/en/microservices-the-architectural-style-for-modern-applications) architecture is hosted that contains all the functionality related to the company's core business.
-   A digital product layer, that is, a set of APIs to easily connect external channels and decrease time-to-market.

![Digital Platform](imgs/Digital-Platform02.svg)

## Platform Engineering: building the foundation of platforms

One of the first concepts to have in mind when discussing digital platforms is **Platform Engineering**, which, according to [Gartner’s definition](https://www.gartner.com/en/articles/what-s-new-in-the-2022-gartner-hype-cycle-for-emerging-technologies), **is the discipline of building and operating self-service internal developer platforms (IDPs) for software delivery and life cycle management**. This term refers to the initial design phase of the platform, which certainly is essential and can be time‑consuming, especially when starting from scratch. But Platform Engineering also includes the implementation phases, which follow immediately after design, and most importantly the maintenance phase, which is the **continuous work of improving, expanding and updating** the platform's functionality.

Cloud platforms are always evolving. New tools, both open source and proprietary software, are being developed every day, and can be integrated to facilitate or extend certain operations. Beyond that, the daily work of [**Platform Engineers**](https://mia-platform.eu/solutions/platform-engineer-devops/) and their teams (also called Platform Team, as we will see later) focuses on **creating workflows and automation logics** that are tailored to their organization’s needs.

Platform Engineering is gaining momentum, and it is considered a very disruptive trend, so much so that many think that it can replace DevOps and SRE. Read this [blog post](https://blog.mia-platform.eu/en/is-platform-engineering-putting-an-end-to-devops-and-sre) to find out more!

## Platform Economy: seizing the value of platforms

The phenomenon of digital platforms, especially cloud platforms, is now so widespread that it has created an entire economy, called the **Platform Economy**. The interesting fact is that the modularity and flexibility provided by cloud platforms enable even small companies to **compete with far more structured competitors**. Indeed, innovative technology companies that pioneered the Platform Economy, such as Uber, Spotify, and Airbnb, are the most successful companies in establishing themselves in the market.

To explore this further, read this article from our blog: [Platform Economy: why you need a modular IT architecture](https://blog.mia-platform.eu/en/platform-economy-why-you-need-a-modular-it-architecture).

## Platform Company: structuring the company around the platform

The main players in the Platform Economy mentioned above are **Platform Companies**, which are companies that structure their business model around a digital platform. Adopting this approach initially requires significant organizational and operational effort, but in the long run it provides flexibility, agility, and the ability to evolve and seize business opportunities as they arise.

Platform Companies are **focused on the customer**, who is at the center of all strategic decisions. This means that the complexities related with technology implementation or business processes should not be perceived externally; most importantly, **internal complexities should not affect the end users and their experience**.

To learn more about this topic, download our free white paper on [Why and how to evolve into a Platform Company](https://resources.mia-platform.eu/en/white-paper-why-and-how-to-evolve-into-a-platform-company).

## Platform Team: improving every day the platform

Once the platform has been created and implemented within the company, it is necessary to **continue working on it on a daily basis** to extend and expand its functionality. This work is usually done by a dedicated team, called the **Platform Team**, consisting of members with different skills and experience to be as cross‑functional as possible.

Customers of the Platform Team are the developers of the company. The purpose of the Platform Team is to ensure that other teams have an experience as frictionless and self‑service as possible when using the platform: for this reason, the Platform Team must focus on all aspects of the software life cycle.

## Developer Platform: simplifying the development experience

**Developer Platforms** (also called Developer Portals) are designed to simplify the work of development teams and improve the [Developer Experience](https://blog.mia-platform.eu/en/how-a-frictionless-developer-experience-improves-software-development). For greater simplicity and efficiency, it may be useful to create two Developer Platforms, dedicated to different teams with different needs: an [Internal Developer Platform](https://blog.mia-platform.eu/en/5-tips-for-implementing-internal-developer-portal-in-your-company) (IDP) for the organization's internal developers and an [External Developer Platform](https://mia-platform.eu/solutions/external-developer-portal/) (EDP) dedicated to external ones.

**The IDP aims to serve the entire IT department of the organization**. Specifically, this tool allows to:

-   Govern all projects in one place with a regulated environment;
-   Industrialize and automate the entire DevOps cycle to increase productivity;
-   Avoid organizational bottlenecks;
-   Self‑serve developers with all the technology they need;
-   Solve multi/hybrid‑cloud complexity with a uniform view;
-   Enhance accountability and reliability of the released software.

Instead, **the EDP is responsible for improving collaboration with developers outside the organization**, typically from partner companies or customers, by providing a self‑service ecosystem of public APIs and documentation. The EDP allows **APIs to be transformed into monetized resources**, treating them as actual products ([API as a Product](https://blog.mia-platform.eu/en/api-as-a-product-why-apis-are-at-the-heart-of-digital-business) - AaaP), as well as to publish services and provide secure access to trial versions of software.

## Platform as a Service (PaaS): enjoying all the benefits with no effort

As it mentioned, a digital platform provides many benefits, especially if it is custom‑built for the organization in which it is used. However, there are several cases where **it is not possible or cost‑effective to build your own platform** from scratch: the effort required in terms of time, resources, and manpower is considerable both during construction from scratch and for daily maintenance once the platform is up and running.

In such cases, there is a way to benefit from all the advantages of a platform without having to devote effort and energy to build and maintain it: this service is called **Platform as a Service (PaaS)**. With a PaaS, companies can **focus on developing their own software code** and better dedicate themselves to their customers, as they will have no concerns on system and infrastructure administration.

Learn more about [Mia‑Platform PaaS](https://docs.mia-platform.eu/docs/paas/overview), exploring all the benefits and the services it offers.

## Conclusion

In this article, we have collected some of the key concepts related to the cloud platform ecosystem, with a focus on the world of Platform Engineering. Our goal was to offer an introductory overview that covers only a small part of a very rich, vast and constantly evolving ecosystem. We did so by creating a **guide that provides the essential tools to begin your own journey into the world of cloud platforms**, a fast‑paced and constantly growing world within which new tools and opportunities are emerging every day.