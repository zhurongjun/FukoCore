# FukoCore

### 主要代码在Core内，包括以下内容
- 容器: Array, Ringqueue, BitArray ,Pool, SparseArray, HashSet, HashMap 
- 基本字符串: 修改时拷贝的字符串，全局唯一的共享字符串 (Name)
- 内存分配器: pmr分配器, 分配堆器, 块分配器 
- 多线程支持: 一个基于共享任务队列的JobSystem, 一个线程池 
- 文件读写: filestreaming 
- 基本算法: 二分查找，内省排序，堆排序，杂耍旋转，编辑距离算法，堆等常见算法 

#### 大多实现来自于Unreal不过均重度魔改(用不惯), 并统一了API，准备将少数容器兼容STL API继续使用
