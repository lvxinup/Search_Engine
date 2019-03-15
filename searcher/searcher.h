#pragma once 
//构建索引模块和搜索模块
#include<string>
#include<vector>
#include<unordered_map>
#include "../jieba/Jieba.hpp"

namespace searcher
{
  struct DocInfo
  {
    uint64_t doc_id;
    std::string title;
    std::string content;
    std::string url;
  };

  //Weight表示的含义是某个词在某个文档中出现过，以及该词出现的权重是多少
  struct Weight
  {
    uint64_t doc_id;
    int weight; //权重，为了后面进行排序做准备
                //当前采用词频计算权重
    std::string key; 
  };

//类型重命名，创建一个倒排拉链类型
typedef std::vector<Weight> InvertedList;

//通过这个类来描述索引模块
class Index
{
private:
  //使用vector下标来表示文档id
  std::vector<DocInfo> forward_index_;
  //知道某个词，获取到对应的 id 列表
  std::unordered_map<std::string,InvertedList> inverted_index_;
  cppjieba::Jieba jieba_;
public:
  //读取raw_input文件，在内存中构建索引
  bool Build(const std::string& input_path);
  //查正排，给定id找到文档内容
  const DocInfo* GetDocInfo(uint64_t doc_id) const;
  //查倒排，给定词，找到这个词在哪些文档中出现过
  const  InvertedList* GetInvertedList(const std::string& key) const;

  void CutWord(const std::string& input,std::vector<std::string>* output);
  
  Index();

private:
  const DocInfo* BuildForward(const std::string& line);
  void BuildInverted(const DocInfo& doc_info);
};

//搜索模块
class Searcher
{
private:
  //当要调用的对象特别大，就不能在栈上创建，需要在堆上创建
  Index* index_;
public:
  Searcher():index_(new Index())
  {}
  ~Searcher()
  {
    delete index_;
  }
  
  //加载索引
  bool Init(const std::string& input_path);
  //通过特定的格式在 result 字符串中表示搜索结果。
  bool Search(const std::string& query,std::string* json_result);

private:
  std::string GetDesc(const std::string& content,const std::string& key);
};

}//end seacher 
