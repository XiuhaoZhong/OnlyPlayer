## 开发阶段记录
 每一次提交的主要内容，以及下一次要继续的内容，同时也包括当前开发阶段的问题以及解决方法

 ### 时间：2022.02.25
 #### 提交概要：
 1、完善CSJVideoFrame，包括右值引用和拷贝引用的支持；
 2、完善ringbuffer，使用unique_ptr管理数据，使数据避免多次拷贝；
 3、定义CSJFFMpegDecoder数据提交接口，主要是CSJDecoderDataDelegate的API，是decoder向syncer中提交数据的关键；

 #### 下一步开发概要：
 1、完成decoder向syncer中数据提交的过程；
 2、完成playerController的关键逻辑，将之前试验中的解码逻辑改成从controller开始的逻辑；

 ### 问题以及解决
 1、ringbuffer模板类在编译时提示未定义方法。通过将类方法的实现放到头文件中解决，这是因为在编译模板类时，编译必须第一时间获取到所有富豪定义和实现，所以像STL中的模板类，都是只有头文件的。

 2、解码音频数据时，对音频数据大小的计算理解错误。对于aac mp3这种压缩格式的音频，解码时通常是一个Packet对应一帧音频数据，所以计算方法如下：
 $$ DataLen = channels * sampleNumber * sizeof(simple) $$
 channels 表示通道数，通常是2，也就是双声道
 sampleNumber表示样本数量，aac一帧里面通常是1024，而mp3通常是1152
 sizeof(sample)就是一个样本的大小，一个样本通常使用uint8_t或者float来表示。

 3、如何实现解码线程的暂停和继续。此处采用了std::mutex 和 std::condition_variable联合的方法，需要暂停时，如下：
           
	std::unique_lock<std::mutex> lock(m_vMutex);
	m_vCondVar.wait(lock);

在 需要继续的时候，进行以下操作：
      
	  m_vCondVar.notify_one();

条件变量的notify_one() 就是通知当前wait的线程可以继续执行，此处notify_one()只是通知一个，还有通知多个的接口，这种情况以后再说。
