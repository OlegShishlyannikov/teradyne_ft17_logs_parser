#ifndef TERADYNE_PARSER_HPP
#define TERADYNE_PARSER_HPP

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

#define NAMEOF( x ) std::string( #x )

class teradyne_parser
{
public:
  
  explicit teradyne_parser( std::string & teradyne_raw_data );
  virtual ~teradyne_parser() = default;

  std::string get_csv_report_table();
  std::vector< std::tuple< int, std::string, std::pair< std::map< std::string, std::string >, std::map< int, std::map< std::string, std::string >>>, std::string, std::string >> * get_metadata();  
  void print_log( std::vector< std::tuple< int, std::string, std::pair< std::map< std::string, std::string >, std::map< int, std::map< std::string, std::string >>>, std::string, std::string >> & teradyne_metadata );
  void print_report( std::vector< std::tuple< int, std::string, std::pair< std::map< std::string, std::string >, std::map< int, std::map< std::string, std::string >>>, std::string, std::string >> & teradyne_metadata );

private:
	
  std::vector< std::string > teradyne_get_blocks( std::string & raw_data );
  std::string teradyne_get_header( std::string & block );
  std::string teradyne_get_body( std::string & block );

  std::string teradyne_parse_datetime( std::string & header );
  std::string teradyne_parse_prog_name( std::string & header );
  std::string teradyne_parse_job_name( std::string & header );
  int teradyne_parse_lot( std::string & header );
  std::string teradyne_parse_operator( std::string & header );
  std::string teradyne_parse_test_mode( std::string & header );
  std::string teradyne_parse_node_name( std::string & header );
  std::string teradyne_parse_part_type( std::string & header );
  std::string teradyne_parse_channel_map( std::string & header );
  std::string teradyne_parse_environment( std::string & header );
  std::string teradyne_parse_family_id( std::string & header );
  std::string teradyne_parse_floor_id( std::string & header );
  std::string teradyne_parse_flow_id( std::string & header );
  std::string teradyne_parse_test_temp( std::string & header );
  std::string teradyne_parse_user_text( std::string & header );

  /* Get test data from body */
  std::map< int, std::map< std::string, std::string >> teradyne_get_tests( std::string & body );
  std::tuple< std::string, std::string > teradyne_check_tests( std::map< int, std::map< std::string, std::string >> & tests_meta );

  std::vector< std::tuple< int, std::string, std::pair< std::map< std::string, std::string >, std::map< int, std::map< std::string, std::string >>>, std::string, std::string >> teradyne_metadata;
};

#endif /* TERADYNE_PARSER_HPP */
