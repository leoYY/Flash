# Flash Stream Computing #
Flash的目的是实现一套暂时类似Heron的实时计算系统。  
相比Storm，Heron结合了现在的集群调度系统，以及加大了Trace方面的考虑。这也正式我之前遇到的问题，在集群规模不断变大的同时，如何trace系统能了一个很大的问题。

整体架构类似Master－Slave模型。
Master负责调度，命名发现，负载均衡以及与调度系统的资源通信。
Slave为实际的交互模块以及业务计算模块。

整体设想：  

－ 用户创建Master任务在集群调度系统中，且Master根据任务topology配置进行申请slave资源进行搭建。  
－ 每个业务模块作为Slave与Master保持心跳，且Master会通知Slave上下游链接关系。

目标：  
－ 支持两种模式，1. 可重不可丢。2. 可丢不可重。且重或丢均尽量避免最小。  
－ 性能吞吐相比简单rpc不会下降过多。
