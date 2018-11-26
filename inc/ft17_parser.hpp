#ifndef FT17_PARSER_HPP
#define FT17_PARSER_HPP

#include <sstream>
#include <iostream>
#include <functional>
#include <algorithm>
#include <vector>
#include <map>
#include <tuple>
#include <regex>
#include <cassert>
#include <QRegularExpression>
#include <QDateTime>

class ft17_parser
{
public:

  explicit ft17_parser( std::string & ft17_raw_data );
  virtual ~ft17_parser() = default;

  std::string get_csv_report_table();
  std::vector< std::tuple< int, std::string, std::pair< std::map< std::string, std::string >, std::map< int, std::map< std::string, std::string >>>, std::string, std::string >> * get_metadata();
  void print_log( std::vector< std::tuple< int, std::string, std::pair< std::map< std::string, std::string >, std::map< int, std::map< std::string, std::string >>>, std::string, std::string >> & ft17_metadata );
  void print_report( std::vector< std::tuple< int, std::string, std::pair< std::map< std::string, std::string >, std::map< int, std::map< std::string, std::string >>>, std::string, std::string >> & ft17_metadata );
  
private:

  std::vector< std::string > ft17_get_blocks( std::string & raw_data );
  std::string ft17_get_header( std::string & block );
  std::string ft17_get_body( std::string & block );

  std::string ft17_parse_prog_name( std::string & header );
  std::string ft17_parse_job_name( std::string & header );
  std::string ft17_parse_part_name( std::string & header );
  std::string ft17_parse_open_datetime( std::string & header );
  std::string ft17_parse_close_datetime( std::string & header );
  std::string ft17_parse_run_number( std::string & header );
  std::string ft17_parse_start_datetime( std::string & header );
  std::string ft17_parse_stop_datetime( std::string & header );
  std::string ft17_parse_operator( std::string & header );
  int ft17_parse_lot( std::string & header );
  std::string ft17_parse_site( std::string & header );
  std::string ft17_parse_result( std::string & header );
  std::string ft17_parse_category( std::string & header );

  std::map< int, std::map< std::string, std::string >> ft17_get_tests( std::string & body );
  std::tuple< std::string, std::string > ft17_check_tests( std::map< int, std::map< std::string, std::string >> & tests_meta );

  std::vector< std::tuple< int, std::string, std::pair< std::map< std::string, std::string >, std::map< int, std::map< std::string, std::string >>>, std::string, std::string >> ft17_metadata;
};

#endif /* FT17_PARSER_HPP */
