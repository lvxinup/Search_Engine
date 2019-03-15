#include "cpp-httplib/httplib.h"
#include <iostream>
#include "../searcher/searcher.h"

int main()
{
  //1.创建searcher对象，并初始化
  searcher::Searcher s;
  bool ret = s.Init("../data/tmp/raw_input");
  if(!ret)
  {
    std::cout<<"searcher Init failed"<<std::endl;
    return -1;
  }
  //2.创建http服务器
  using namespace httplib;
  Server server;
  //[&s] 是lambda表达式获取文件中的对象，并且以引用的方式获取
  // /search?query=filesystem
  std::cout<<"开始创建http服务器"<<std::endl;
  server.Get("/search",[&s](const Request& req,Response& res){
      std::string query = req.get_param_value("query");
      std::string result;
      s.Search(query,&result);
      res.set_content(result,"text/plain");
      });
  server.listen("0.0.0.0",8080);

  return 0;
}
