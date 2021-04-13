---
title: "linux5.3.0编译运行LINUX内核源码中的BPF示例代码"
date: 2021-03-31T08:45:20+08:00
tags: ["kernel", "ebpf"]
categories: ["kernel", "ebpf"]
banner: "img/banners/ms.jpg"
author: "helight"
authorlink: "http://helight.info"
summary: ""
keywords: ["kernel","ebpf"]
draft: true
---

## 前言


## minikube install

在 linux 上安装 minikube
在linux上怎么安装minikube看这里：
https://minikube.sigs.k8s.io/docs/start/linux/.
很简单，下载安装即可

 curl -LO https://storage.googleapis.com/minikube/releases/latest/minikube-linux-amd64 \
   && sudo install minikube-linux-amd64 /usr/local/bin/minikube

### 启动过程中的一些错误
#### hostname 配置问题
W0414 11:15:16.799163 11416 configset.go:202] WARNING: kubeadm cannot validate component configs for API groups [kubelet.config.k8s.io kubeproxy.config.k8s.io]nodeRegistration.name: Invalid value: "vm_74_51_centos": a DNS-1123 subdomain must consist of lower case alphanumeric characters, '-' or '.', and must start and end with an alphanumeric character (e.g. 'example.com', regex used for validation is '[a-z0-9]([-a-z0-9]*[a-z0-9])?(\.[a-z0-9]([-a-z0-9]*[a-z0-9])?)*')To see the stack trace of this error execute with --v=5 or higher

[root@VM_74_51_centos /data/minikube]# hostnamectl set-hostname vm-74-51-centos

#### conntrack-tools 没有安装
* minikube v1.9.2 on Centos 7.2
* Using the none driver based on user configuration
X Sorry, Kubernetes v1.18.0 requires conntrack to be installed in root's path

[root@VM_74_51_centos /data/minikube]# apt-get install conntrack

## kubectl install 
curl -LO https://storage.googleapis.com/kubernetes-release/release/`curl -s https://storage.googleapis.com/kubernetes-release/release/stable.txt`/bin/linux/amd64/kubectl
install kubectl /usr/local/bin/kubectl

## docker install
apt-get remove docker docker-engine docker.io containerd runc

### Update the apt package index and install packages to allow apt to use a repository over HTTPS:
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
### Add Docker’s official GPG key:

 curl -fsSL https://download.docker.com/linux/ubuntu/gpg | sudo gpg --dearmor -o /usr/share/keyrings/docker-archive-keyring.gpg

## dashboard install
### nvm 安装
curl -o- https://raw.githubusercontent.com/nvm-sh/nvm/v0.38.0/install.sh | bash
bash: source ~/.bashrc
### 更新node
nvm install node # "node" is an alias for the latest version

### Install the gulp command line utility
npm install --global gulp-cli

golang install
https://golang.org/dl/

## create token
kubectl create namespace kubernetes-dashboard
namespace/kubernetes-dashboard created
root@VM-74-51-ubuntu:/data/k8s# cat <<EOF | kubectl apply -f -
apiVersion: v1
kind: ServiceAccount
metadata:
  name: admin-user
  namespace: kubernetes-dashboard
EOF

serviceaccount/admin-user created


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

eyJhbGciOiJSUzI1NiIsImtpZCI6Imt4T1pLUG4zRW5LQTAxcFhlUVJBX1BUUnE4ZWRpa3ZkMWxHOVEwQ0hjYlEifQ.eyJpc3MiOiJrdWJlcm5ldGVzL3NlcnZpY2VhY2NvdW50Iiwia3ViZXJuZXRlcy5pby9zZXJ2aWNlYWNjb3VudC9uYW1lc3BhY2UiOiJrdWJlcm5ldGVzLWRhc2hib2FyZCIsImt1YmVybmV0ZXMuaW8vc2VydmljZWFjY291bnQvc2VjcmV0Lm5hbWUiOiJhZG1pbi11c2VyLXRva2VuLXptanhzIiwia3ViZXJuZXRlcy5pby9zZXJ2aWNlYWNjb3VudC9zZXJ2aWNlLWFjY291bnQubmFtZSI6ImFkbWluLXVzZXIiLCJrdWJlcm5ldGVzLmlvL3NlcnZpY2VhY2NvdW50L3NlcnZpY2UtYWNjb3VudC51aWQiOiIyZGE1MTM2Ny00YzE1LTQ2ODgtODBiMi04NDNhMjJkMDIzM2IiLCJzdWIiOiJzeXN0ZW06c2VydmljZWFjY291bnQ6a3ViZXJuZXRlcy1kYXNoYm9hcmQ6YWRtaW4tdXNlciJ9.EsUY8dmQkuDu4TdTAKeqF6orNt28msrGNGiff0ACCHcpQdB7ISyyaydoWmCF409pKArqhdeWsHZqCaIYZ-0iumRZj2O16vp8u9_efGhAXXUWuPzPKZyxsoA1hrgZK6H-8MPtRijVpx7AkeAR_Bf7EUmEGDOig8LOeVppdh7lZ9fmUx89xiqFvg7YEJfTs1naRWryvWDgw0FTVRsnS_-cpACR0WpiCdcSs2zGhvuT9DrwDHTMQYOplVybGNfPc8V7wU37JijA7RR4fUFucprqh2MgOyOfASPt70ZVJup8jPTOhb_MkjcbWtoNBgs4BHbRzt35rtn5Xkggd8O4PJtpSQ
##
start
```sh
export GOPATH=$HOME/go
npm run start:prod --kubernetes-dashboard:bind_address="0.0.0.0" --enable-skip-login
```
npm run start:https --kubernetes-dashboard:kubeconfig="/root/.kube/config" --kubernetes-dashboard:bind_address="0.0.0.0"

./dist/amd64/dashboard --kubeconfig /root/.kube/config --auto-generate-certificates --bind-address "0.0.0.0"


<center>
看完本文有收获？请分享给更多人

关注「黑光技术」，关注大数据+微服务

![](/img/qrcode_helight_tech.jpg)
</center>