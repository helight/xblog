---
title: "k8s代码走读---client-go编程交互代码测试"
date: 2020-08-20T08:45:20+08:00
tags: ["k8s", "microservices"]
categories: ["k8s", "microservices"]
banner: "img/banners/kubernetes.jpeg"
author: "helight"
authorlink: "http://helight.info"
summary: ""
keywords: ["k8s","microservices"]
draft: false
---

## 前言
这部分还是以 client-go 为基础的代码测试，今天这里分享的一个是我测试过的 client-go 代码例子，
``` golang
package main

import (
  "context"
  "flag"
  "fmt"
  "log"
  "path/filepath"

  metav1 "k8s.io/apimachinery/pkg/apis/meta/v1"
  "k8s.io/client-go/kubernetes"
  "k8s.io/client-go/tools/clientcmd"
  "k8s.io/client-go/util/homedir"
)

var clientset *kubernetes.Clientset

func main() {
  var kubeconfig *string
  // 获取配置文件路径
  if home := homedir.HomeDir(); home != "" {
    kubeconfig = flag.String("kubeconfig", filepath.Join(home, ".kube", "config"), "(optional) absolute path to the kubeconfig file")
  } else {
    kubeconfig = flag.String("kubeconfig", "", "absolute path to the kubeconfig file")
  }
  flag.Parse() // flags解析
  config, err := clientcmd.BuildConfigFromFlags("", *kubeconfig) // 根据配置文件生成配置文件对象，这个对象如下面的代码注释
  if err != nil {
    log.Println(err)
    return
  }
  clientset, err = kubernetes.NewForConfig(config) // 根据配置文件对象构建客户端
  if err != nil {
    log.Fatalln(err)
    return
  } else {
    fmt.Println("connect k8s success")
  }

  pods, err := clientset.CoreV1().Pods("").List(context.TODO(), metav1.ListOptions{}) // 获取 pods 列表信息
  if err != nil {
    log.Println(err.Error())
    return
  }

  fmt.Println(pods.Items[1].Name)
  fmt.Println(pods.Items[1].CreationTimestamp)
  fmt.Println(pods.Items[1].Labels)
  fmt.Println(pods.Items[1].Namespace)
  fmt.Println(pods.Items[1].Status.HostIP)
  fmt.Println(pods.Items[1].Status.PodIP)
  fmt.Println(pods.Items[1].Status.StartTime)
  fmt.Println(pods.Items[1].Status.Phase)
  fmt.Println(pods.Items[1].Status.ContainerStatuses[0].RestartCount) //重启次数
  fmt.Println(pods.Items[1].Status.ContainerStatuses[0].Image)        //获取重启时间

  fmt.Println("##################")
  nodes, err := clientset.CoreV1().Nodes().List(context.TODO(), metav1.ListOptions{}) // 获取节点列表信息
  fmt.Println(nodes.Items[0].Name)
  fmt.Println(nodes.Items[0].CreationTimestamp) //加入集群时间
  fmt.Println(nodes.Items[0].Status.NodeInfo)
  fmt.Println(nodes.Items[0].Status.Conditions[len(nodes.Items[0].Status.Conditions)-1].Type)
  fmt.Println(nodes.Items[0].Status.Allocatable.Memory().String())
}
```
Clientset 这个对象可以看一下，可以看出这个对象是一些不同版本客户端对象的集合。
``` golang
type Clientset struct {
	*discovery.DiscoveryClient
	admissionregistrationV1      *admissionregistrationv1.AdmissionregistrationV1Client
	admissionregistrationV1beta1 *admissionregistrationv1beta1.AdmissionregistrationV1beta1Client
	appsV1                       *appsv1.AppsV1Client
	appsV1beta1                  *appsv1beta1.AppsV1beta1Client
	appsV1beta2                  *appsv1beta2.AppsV1beta2Client
	authenticationV1             *authenticationv1.AuthenticationV1Client
	authenticationV1beta1        *authenticationv1beta1.AuthenticationV1beta1Client
	authorizationV1              *authorizationv1.AuthorizationV1Client
  authorizationV1beta1         *authorizationv1beta1.AuthorizationV1beta1Client
  ...
}
```
下面是这个是配置文件的对象。
```golang
type Config struct {
	Host string // host:port 对，或者是一个 apiserver 的 URL。
	APIPath string // 是用于访问 apiserver 的一个 API 前缀。
	ContentConfig // 包含了对象转换的配置信息
	Username string // 基本鉴权需要的用户名和密码
	Password string
	BearerToken string // Bearer 认证需要的信息
	BearerTokenFile string // 包含 BearerToken 的文件

	// Impersonate is the configuration that RESTClient will use for impersonation.
	Impersonate ImpersonationConfig

	// Server requires plugin-specified authentication.
	AuthProvider *clientcmdapi.AuthProviderConfig

	// Callback to persist config for AuthProvider.
	AuthConfigPersister AuthProviderConfigPersister

	// Exec-based authentication provider.
	ExecProvider *clientcmdapi.ExecConfig

	// TLSClientConfig contains settings to enable transport layer security
	TLSClientConfig

	// UserAgent is an optional field that specifies the caller of this request.
	UserAgent string

	// DisableCompression bypasses automatic GZip compression requests to the
	// server.
	DisableCompression bool

	// Transport may be used for custom HTTP behavior. This attribute may not
	// be specified with the TLS client certificate options. Use WrapTransport
	// to provide additional per-server middleware behavior.
	Transport http.RoundTripper
	// WrapTransport will be invoked for custom HTTP behavior after the underlying
	// transport is initialized (either the transport created from TLSClientConfig,
	// Transport, or http.DefaultTransport). The config may layer other RoundTrippers
	// on top of the returned RoundTripper.
	//
	// A future release will change this field to an array. Use config.Wrap()
	// instead of setting this value directly.
	WrapTransport transport.WrapperFunc

	// QPS indicates the maximum QPS to the master from this client.
	// If it's zero, the created RESTClient will use DefaultQPS: 5
	QPS float32

	// Maximum burst for throttle.
	// If it's zero, the created RESTClient will use DefaultBurst: 10.
	Burst int

	// Rate limiter for limiting connections to the master from this client. If present overwrites QPS/Burst
	RateLimiter flowcontrol.RateLimiter

	// WarningHandler handles warnings in server responses.
	// If not set, the default warning handler is used.
	WarningHandler WarningHandler

	// The maximum length of time to wait before giving up on a server request. A value of zero means no timeout.
	Timeout time.Duration

	// Dial specifies the dial function for creating unencrypted TCP connections.
	Dial func(ctx context.Context, network, address string) (net.Conn, error)

	// Proxy is the the proxy func to be used for all requests made by this
	// transport. If Proxy is nil, http.ProxyFromEnvironment is used. If Proxy
	// returns a nil *URL, no proxy is used.
	//
	// socks5 proxying does not currently support spdy streaming endpoints.
	Proxy func(*http.Request) (*url.URL, error)

	// Version forces a specific version to be used (if registered)
	// Do we need this?
	// Version string
}

```