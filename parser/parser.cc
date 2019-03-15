//数据处理模块
//处理boost文档中的html文件：
//1.去标签
//2.把文件进行合并,合并成一个行文本文件
//3.对文档的结构进行分析，提取出标题，正文，目标url

#include <string>
#include <iostream>
#include <vector>
//遍历和枚举目录
#include <boost/filesystem/path.hpp>
#include <boost/filesystem/operations.hpp>
#include "../common/util.hpp"
#include <fstream>
#include <unistd.h>

const std::string input_path = "../data/input/html/";
const std::string output_path = "../data/tmp/raw_input";

//doc指的是文档，待搜索的html
struct DocInfo
{
  std::string title;
  std::string content;
  std::string url;
};
bool EnumFile(const std::string& input_path,std::vector<std::string>* file_list)
{
  namespace fs = boost::filesystem;
  //input_path是一个字符串，根据这个字符串构造出一个path对象
  fs::path root_path(input_path);
  if(!fs::exists(root_path))
  {
    std::cout<<"input_path not exist! input_path="<<input_path<<std::endl;
    return false;
  }
  //boost递归遍历目录，借助一个特殊迭代器
  fs::recursive_directory_iterator end_iter;
  for(fs::recursive_directory_iterator iter(root_path);iter!=end_iter;iter++)
  {
    
    //剔除目录，根据扩展名，只保留html
    if(!fs::is_regular_file(*iter))  //选择普通文件的函数
    {

      continue;
    }

    if(iter->path().extension()!=".html") //只保留html
    {

      continue;
    }

    file_list->push_back(iter->path().string());
  
  }
  return true;
}
bool ParseTitle(const std::string& html,std::string* title)
{
  //1.先查找<title>标签
  size_t begin = html.find("<title>");
  if(begin == std::string::npos)
  {
    std::cout<<"title not found!"<<std::endl;
    return false;
  }
  //2.在查找</title>标签
  size_t end = html.find("</title>");
  if(end == std::string::npos)
  {
    std::cout<<"/title not found!"<<std::endl;
    return false;
  }
  //3.通过字符串取子串的方式获取到title标签中的内容
  begin += std::string("<title>").size();
  if(begin>end) //标题可能为空
  {
    std::cout<<"begin end error"<<std::endl;
    return false;
  }
  *title = html.substr(begin,end-begin);
  return true;
}
bool ParseContent(const std::string& html,std::string* content)
{
  //一个字符一个字符读取
  //如果当前字符是正文内容，就写入结果
  //如果当前字符是<认为标签开始，接下来的字符就会舍弃，一直遇到>认为标签结束，接下来的字符就恢复
  
  
  //这个变量为true，代表处理正文，否则代表处理标签
  bool is_content = true;
  for(auto c : html )
  { 
    if(is_content)
    {
      //当前为正文状态
      if(c == '<')  //进入标签状态
      {
        is_content = false;
        continue;
      }  
      else 
      { //当前字符就是普通的正文字符，需要加入到结果中
        if(c == '\n')
        {
          c = ' ';//行文本文件，将换行替换成空格。
        }
        content->push_back(c);
      }
    }
    else
    {
      //当前是标签状态
      if(c=='>')
      {
        is_content = true;
      }
    }
  }
  return true;
}
//boost 文档url有一个统一的前缀
//https://www.boost.org/doc/libs/1_53_0/doc/
//URL的后半部分可以通过该文档的路径中解析出来
//文档的路径形如 ../data/input/  html/thread.htmml
bool ParseUrl(const std::string& file_path,std::string* url )
{
  std::string prefix = "https://www.boost.org/doc/libs/1_53_0/doc/html/"; 
  std::string tail = file_path.substr(input_path.size());
  *url = prefix +tail;
  return true;
}

bool ParseFile(const std::string& file_path,DocInfo* doc_info)
{ 
  //1.打开文件，读取文件内容
  std::string html;
  bool ret = FileUtil::Read(file_path,&html);
  if(!ret)
  {
    std::cout<<"Read file failed! file_path="<<file_path<<std::endl;
  }
  //2.解析标题
  ret = ParseTitle(html,&doc_info->title);
  if(!ret)
  {
    std::cout<<"ParseTitle failed!"<<file_path<<std::endl;
    return false;
  }
  //3.解析正文，并且去除html标签
  ret = ParseContent(html,&doc_info->content);
  if(!ret)
  {
    std::cout<<"ParseContent failed! file_path="<<file_path<<std::endl;
    return false;
  }
  //4.解析出url
  ret = ParseUrl(file_path,&doc_info->url);
  if(!ret)
  {
    std::cout<<"ParseUrl failed! file_path="<<file_path<<std::endl;
    return false;
  }
  return true;
}
//C++中的iostream和fstream等这些对象都是禁止拷贝的
//最终的输出结果是一个行文本文件，每一行对应一个 html 文档。
//也就是每一行对应一个doc_info
void WriteOutput(const DocInfo& doc_info,std::ofstream& file)
{
  std::string line = doc_info.title + "\3"+ doc_info.url +"\3" + doc_info.content + "\n";
  file.write(line.c_str(),line.size());
}

int main()
{
  //1.枚举出输入路径中所有的 html 文档的路径
    //vector每个元素是一个文件的路径
  std::vector<std::string> file_list;
  bool ret = EnumFile(input_path,&file_list);
  if(!ret)
  {
    std::cout<<"EnumFile error "<<std::endl;
    return -1;
  } 
  for(const auto& str : file_list)
  {
    std::cout<<str<<std::endl;
  }

  //打开输出文件，存放doc_info
  if(0==access(output_path.c_str(),F_OK)) //如果文件可以打开，返回0
  {
     std::cout<<"access success"<<std::endl;
  }
  std::ofstream output_file(output_path.c_str()); 
  if(!output_file.is_open())
  {
    std::cout<<"open output_file failed! output_path = "<<output_path<<std::endl;
     return -1;
  }
  //2.依次处理枚举出的路径，对该文件进行分析，去标签，解析标题/正文/url 
  for(const auto& file_path : file_list)
  {
     DocInfo info;
     //输入要解析的文件路径，输出得到的 DocInfo结构。
     ret = ParseFile(file_path,&info);
     if(!ret)
     {
       std::cout<<"ParseFile failed ! file_path="<<file_path<<std::endl;
       //return -1;
       continue;
     }
     
     //3.分析结果写入行文本文件
     WriteOutput(info,output_file);
  }

  output_file.close();
  return 0;
}
