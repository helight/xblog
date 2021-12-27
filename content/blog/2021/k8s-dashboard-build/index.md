---
title: "k8s dashboard 编译测试"
date: 2021-03-31T08:45:20+08:00
tags: ["k8s"]
categories: ["k8s"]
banner: "img/banners/kubernetes.jpeg"
author: "helight"
authorlink: "http://helight.cn"
summary: ""
keywords: ["k8s","dashboard"]
draft: false
---

## 前言


## minikube install

在 linux 上安装 minikube
在linux上怎么安装minikube看这里：
https://minikube.sigs.k8s.io/docs/start/linux/.
很简单，下载安装即可

```sh
 curl -LO https://storage.googleapis.com/minikube/releases/latest/minikube-linux-amd64 \
   && sudo install minikube-linux-amd64 /usr/local/bin/minikube
``
### 启动过程中的一些错误
#### hostname 配置问题
```sh
W0414 11:15:16.799163 11416 configset.go:202] WARNING: kubeadm cannot validate component configs for API groups [kubelet.config.k8s.io kubeproxy.config.k8s.io]nodeRegistration.name: Invalid value: "vm_74_51_centos": a DNS-1123 subdomain must consist of lower case alphanumeric characters, '-' or '.', and must start and end with an alphanumeric character (e.g. 'example.com', regex used for validation is '[a-z0-9]([-a-z0-9]*[a-z0-9])?(\.[a-z0-9]([-a-z0-9]*[a-z0-9])?)*')To see the stack trace of this error execute with --v=5 or higher
```
解决方法
```sh
[root@VM_74_51_centos /data/minikube]# hostnamectl set-hostname vm-74-51-centos
```
#### conntrack-tools 没有安装
```sh
* minikube v1.9.2 on Centos 7.2
* Using the none driver based on user configuration
X Sorry, Kubernetes v1.18.0 requires conntrack to be installed in root's path
```
解决方法
```sh
[root@VM_74_51_centos /data/minikube]# apt-get install conntrack
```
## kubectl install 
```sh
curl -LO https://storage.googleapis.com/kubernetes-release/release/`curl -s https://storage.googleapis.com/kubernetes-release/release/stable.txt`/bin/linux/amd64/kubectl
install kubectl /usr/local/bin/kubectl
```
## docker install
```sh
apt-get remove docker docker-engine docker.io containerd runc

apt-get update
 sudo apt-get install \
    apt-transport-https \
    ca-certificates \
    curl \
    gnupg \
    lsb-release

echo \
  "deb [arch=amd64 signed-by=/usr/share/keyrings/docker-archive-keyring.gpg] https://download.docker.com/linux/ubuntu \
  $(lsb_release -cs) stable" | sudo tee /etc/apt/sources.list.d/docker.list > /dev/null

 sudo apt-get update
 sudo apt-get install docker-ce docker-ce-cli containerd.io
 ```
### Add Docker’s official GPG key:
```sh
 curl -fsSL https://download.docker.com/linux/ubuntu/gpg | sudo gpg --dearmor -o /usr/share/keyrings/docker-archive-keyring.gpg
```
## dashboard install
### nvm 安装
```sh
curl -o- https://raw.githubusercontent.com/nvm-sh/nvm/v0.38.0/install.sh | bash
bash: source ~/.bashrc
```
### 更新node
``` sh
nvm install v12.22.0
```
要使用npm6的版本，7的版本编译
```sh
ubuntu@VM-74-51-ubuntu:/data/dashboard$ npm version
{
  'kubernetes-dashboard': '2.2.0',
  npm: '6.14.11',
  ares: '1.16.1',
  brotli: '1.0.9',
  cldr: '37.0',
  http_parser: '2.9.4',
  icu: '67.1',
  llhttp: '2.1.3',
  modules: '72',
  napi: '8',
  nghttp2: '1.41.0',
  node: '12.22.0',
  openssl: '1.1.1j',
  tz: '2019c',
  unicode: '13.0',
  uv: '1.40.0',
  v8: '7.8.279.23-node.46',
  zlib: '1.2.11'
}
ubuntu@VM-74-51-ubuntu:/data/dashboard$ 
```
### Install the gulp command line utility
npm install --global gulp-cli

### golang install
https://golang.org/dl/

## create token
```sh
kubectl create namespace kubernetes-dashboard
namespace/kubernetes-dashboard created
```
创建账号
```sh
root@VM-74-51-ubuntu:/data/k8s# cat <<EOF | kubectl apply -f -
apiVersion: v1
kind: ServiceAccount
metadata:
  name: admin-user
  namespace: kubernetes-dashboard
EOF

serviceaccount/admin-user created
```
绑定角色
```sh
cat <<EOF | kubectl apply -f -
apiVersion: rbac.authorization.k8s.io/v1
kind: ClusterRoleBinding
metadata:
  name: admin-user
roleRef:
  apiGroup: rbac.authorization.k8s.io
  kind: ClusterRole
  name: cluster-admin
subjects:
- kind: ServiceAccount
  name: admin-user
  namespace: kubernetes-dashboard
EOF
```

## start
```sh
export GOPATH=$HOME/go
npm run start --kubernetes-dashboard:bind_address="0.0.0.0" --enable-skip-login
```

<center>
看完本文有收获？请分享给更多人

关注「黑光技术」，关注大数据+微服务

![](/img/qrcode_helight_tech.jpg)
</center>