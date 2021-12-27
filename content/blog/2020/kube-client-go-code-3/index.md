---
title: "k8s 代码走读---client-go 编程交互代码测试"
date: 2020-08-20T08:45:20+08:00
tags: ["k8s", "microservices"]
categories: ["k8s", "microservices"]
banner: "img/banners/kubernetes.jpeg"
author: "helight"
authorlink: "http://helight.cn"
summary: "这部分还是以 client-go 为基础的代码测试，今天这里分享的一个是我测试过的 client-go 代码例子"
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

	for index, pod := range pods.Items {
		fmt.Println("pods info", index)
		fmt.Println("pods info", pod.Name)
		fmt.Println("pods info", pod.CreationTimestamp)
		fmt.Println("pods info", pod.Labels)
		fmt.Println("pods namespace", pods.Items[1].Namespace)
		fmt.Println("pods info", pod.Status.HostIP)
		fmt.Println("pods info", pod.Status.PodIP)
		fmt.Println("pods info", pod.Status.StartTime)
		fmt.Println("pods info", pod.Status.Phase)
		fmt.Println("pods info", pod.Status.ContainerStatuses[0].RestartCount) //重启次数
		fmt.Println("pods info", pod.Status.ContainerStatuses[0].Image)        //获取重启时间
	}
	fmt.Println("##################")
	nodes, err := clientset.CoreV1().Nodes().List(context.TODO(), metav1.ListOptions{})
	for index, node := range nodes.Items {
		fmt.Println("index info", index)
		fmt.Println("node info ", node.Name)
		fmt.Println("node info ", node.CreationTimestamp) //加入集群时间
		fmt.Println("node info ", node.Status.NodeInfo)
		fmt.Println("node info ", node.Status.Conditions[len(nodes.Items[0].Status.Conditions)-1].Type)
		fmt.Println("node info ", node.Status.Allocatable.Memory().String())
	}
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
	Impersonate ImpersonationConfig
	AuthProvider *clientcmdapi.AuthProviderConfig
	AuthConfigPersister AuthProviderConfigPersister
	ExecProvider *clientcmdapi.ExecConfig
	TLSClientConfig //
	UserAgent string
	DisableCompression bool // 是否开启 GZip 压缩
	Transport http.RoundTripper //
	WrapTransport transport.WrapperFunc //
	QPS float32 // client 访问的最大 QPS，默认是 5
	Burst int  // 最大突发请求量，默认是 10
	RateLimiter flowcontrol.RateLimiter // 这个 client 到 master 的最大请求速率
	WarningHandler WarningHandler // 处理服务响应的告警事件
	Timeout time.Duration // server 访问的超时时间，0 表示不超时
	Dial func(ctx context.Context, network, address string) (net.Conn, error) // 非加密 tcp 链接探测
	Proxy func(*http.Request) (*url.URL, error) // 代理
}
```

## 编译
默认使用 `go mod` 编译，但是 `go mod` 默认获取的 client-go 版本有问题，需要自己调整，才能编译过。我使用的 go.mod 文件。

``` console
module k8sclientx

go 1.13

require (
        github.com/imdario/mergo v0.3.11 // indirect
        golang.org/x/oauth2 v0.0.0-20200107190931-bf48bf16ab8d // indirect
        golang.org/x/time v0.0.0-20200630173020-3af7569d3a1e // indirect
        k8s.io/apimachinery v0.18.8
        k8s.io/client-go v0.18.8
        k8s.io/utils v0.0.0-20200815180417-3bc9d57fc792 // indirect
)
```
输出结果：
``` console
[root@vm-74-51-centos /data/k8s/client-test]# ./k8sclient 
connect k8s success
pods info 0
pods info grafana-54b54568fc-qzv46
pods info 2020-06-02 18:07:11 +0800 CST
pods info map[app:grafana chart:grafana heritage:Tiller pod-template-hash:54b54568fc release:istio-system]
pods namespace istio-system
pods info 9.134.74.51
pods info 192.168.10.8
pods info 2020-06-02 18:07:11 +0800 CST
pods info Running
pods info 1
pods info grafana/grafana:6.5.2
pods info 1
pods info istio-egressgateway-5cbb74b6d-8xskg
pods info 2020-06-02 18:07:10 +0800 CST
pods info map[app:istio-egressgateway chart:gateways heritage:Tiller istio:egressgateway pod-template-hash:5cbb74b6d release:istio service.istio.io/canonical-name:istio-egressgateway service.istio.io/canonical-revision:latest]
pods namespace istio-system
pods info 9.134.74.51
pods info 192.168.10.6
......
```

## 总结

通过上面的分析和示例代码的编译，基本上我掌握了 kubernetes 的 client 编写方式和编译方式，这里比较坑的一点就在编译。默认 gomod 的编译引入的库是有版本冲突的，需要核对使用的版本并修改 `go.mod` 中的相应版本号才能编译通过。