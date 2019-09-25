#include "ft17_parser.hpp"

ft17_parser::ft17_parser(std::string &ft17_raw_data) {
  std::vector<
      std::tuple<int, std::string,
                 std::pair<std::map<std::string, std::string>, std::map<int, std::map<std::string, std::string>>>,
                 std::string, std::string>>
      ft17_metadata;
  std::vector<std::string> blocks = this->ft17_get_blocks(ft17_raw_data);

  for (std::string block : blocks) {

    std::map<std::string, std::string> header_metadata;
    std::string header = this->ft17_get_header(block);
    std::string body = this->ft17_get_body(block);

    std::string prog_name = this->ft17_parse_prog_name(header);
    std::string job_name = this->ft17_parse_job_name(header);
    std::string part_name = this->ft17_parse_part_name(header);
    std::string open_datetime = this->ft17_parse_open_datetime(header);
    std::string close_datetime = this->ft17_parse_close_datetime(header);
    std::string run_number = this->ft17_parse_run_number(header);
    std::string start_datetime = this->ft17_parse_start_datetime(header);
    std::string stop_datetime = this->ft17_parse_stop_datetime(header);
    std::string operator_name = this->ft17_parse_operator(header);
    int lot = this->ft17_parse_lot(header);
    std::string site = this->ft17_parse_site(header);
    std::string result = this->ft17_parse_result(header);
    std::string category = this->ft17_parse_category(header);

    if (job_name == "ЭТТдо")
      job_name = "BEFORE_ETT";
    else if (job_name == "ЭТТпосле")
      job_name = "AFTER_ETT";
    else if (job_name == "Т-")
      job_name = "MINUS";
    else if (job_name == "Т+")
      job_name = "PLUS";

    header_metadata.insert(std::make_pair("prog_name", prog_name));
    header_metadata.insert(std::make_pair("job_name", job_name));
    header_metadata.insert(std::make_pair("part_name", part_name));
    header_metadata.insert(std::make_pair("open_datetime", open_datetime));
    header_metadata.insert(std::make_pair("close_datetime", close_datetime));
    header_metadata.insert(std::make_pair("run_number", run_number));
    header_metadata.insert(std::make_pair("start_datetime", start_datetime));
    header_metadata.insert(std::make_pair("stop_datetime", stop_datetime));
    header_metadata.insert(std::make_pair("operator_name", operator_name));
    header_metadata.insert(std::make_pair("lot", std::to_string(lot)));
    header_metadata.insert(std::make_pair("site", site));
    header_metadata.insert(std::make_pair("result", result));
    header_metadata.insert(std::make_pair("category", category));

    std::map<int, std::map<std::string, std::string>> tests_metadata = this->ft17_get_tests(body);
    std::tuple<std::string, std::string> tests_result = this->ft17_check_tests(tests_metadata);
    std::tuple<int, std::string,
               std::pair<std::map<std::string, std::string>, std::map<int, std::map<std::string, std::string>>>,
               std::string, std::string>
        lot_meta = {lot, job_name, std::make_pair(header_metadata, tests_metadata), std::get<0>(tests_result),
                    std::get<1>(tests_result)};

    bool exists = false;
    int pos = 0;
    for (unsigned int i = 0; i < ft17_metadata.size(); i++) {

      if ((std::get<0>(lot_meta) == std::get<0>(ft17_metadata[i])) &&
          (std::get<1>(lot_meta) == std::get<1>(ft17_metadata[i]))) {

        exists = true;
        pos = i;
        break;

      } else {

        exists = false;
        pos = 0;
        continue;
      }
    }

    if (exists) {

      if (QDateTime::fromString(QString::fromStdString(std::get<2>(lot_meta).first.at("start_datetime")),
                                "dd.MM.yyyy HH:mm:ss")
              .secsTo(QDateTime::fromString(
                  QString::fromStdString(std::get<2>(ft17_metadata[pos]).first.at("start_datetime")),
                  "dd.MM.yyyy HH:mm:ss")) <= 0) {

        ft17_metadata[pos] = lot_meta;
      }

    } else
      ft17_metadata.push_back(lot_meta);
  }

  this->ft17_metadata = ft17_metadata;
}

std::vector<std::tuple<int, std::string,
                       std::pair<std::map<std::string, std::string>, std::map<int, std::map<std::string, std::string>>>,
                       std::string, std::string>> *
ft17_parser::get_metadata() {
  return &this->ft17_metadata;
}

void ft17_parser::print_log(
    std::vector<
        std::tuple<int, std::string,
                   std::pair<std::map<std::string, std::string>, std::map<int, std::map<std::string, std::string>>>,
                   std::string, std::string>> &ft17_metadata) {
  for (unsigned int i = 0; i < ft17_metadata.size(); i++) {

    std::printf(
        "Lot#%i, Job Name : %s, Result : %s, Reason : %s, Tests count : %lu\r\n Header : {"
        "\r\n\tprog_name : %s,\r\n\tjob_name : %s,\r\n\tpart_name : %s\r\n\topen_datetime : %s,\r\n\tclose_datetime : "
        "%s,"
        "\r\n\trun_number : %s,\r\n\tstart_datetime : %s,\r\n\tstop_datetime : %s,\r\n\toperator_name : %s,\r\n\tlot : "
        "%s,"
        "\r\n\tsite : %s,\r\n\tresult : %s,\r\n\tcategory : %s\r\n}\r\n",
        std::get<0>(ft17_metadata[i]), std::get<1>(ft17_metadata[i]).c_str(), std::get<3>(ft17_metadata[i]).c_str(),
        std::get<4>(ft17_metadata[i]).c_str(), std::get<2>(ft17_metadata[i]).second.size(),
        std::get<2>(ft17_metadata[i]).first.at("prog_name").c_str(),
        std::get<2>(ft17_metadata[i]).first.at("job_name").c_str(),
        std::get<2>(ft17_metadata[i]).first.at("part_name").c_str(),
        std::get<2>(ft17_metadata[i]).first.at("open_datetime").c_str(),
        std::get<2>(ft17_metadata[i]).first.at("close_datetime").c_str(),
        std::get<2>(ft17_metadata[i]).first.at("run_number").c_str(),
        std::get<2>(ft17_metadata[i]).first.at("start_datetime").c_str(),
        std::get<2>(ft17_metadata[i]).first.at("stop_datetime").c_str(),
        std::get<2>(ft17_metadata[i]).first.at("operator_name").c_str(),
        std::get<2>(ft17_metadata[i]).first.at("lot").c_str(), std::get<2>(ft17_metadata[i]).first.at("site").c_str(),
        std::get<2>(ft17_metadata[i]).first.at("result").c_str(),
        std::get<2>(ft17_metadata[i]).first.at("category").c_str());

    for (std::map<int, std::map<std::string, std::string>>::iterator it = std::get<2>(ft17_metadata[i]).second.begin();
         it != std::get<2>(ft17_metadata[i]).second.end(); it++) {

      std::printf(
          "Test#%i : {\r\n\ttest_time : %s,\r\n\ttest_name_number : %s,\r\n\ttest_name : %s,\r\n\ttest_result : "
          "%s,\r\n\ttest_pattern : %s,\r\n\ttest_fails : %s"
          ",\r\n\ttest_pin : %s,\r\n\ttest_measured : %s,\r\n\ttest_minumum : %s,\r\n\ttest_maximum : "
          "%s,\r\n\ttest_measurement_unit : %s,\r\n\ttest_site : %s"
          ",\r\n\ttest_force : %s,\r\n\ttest_force_unit : %s,\r\n\ttest_channel : %s,\r\n\t'test_key : %s }\r\n",
          it->first, it->second.at("test_time").c_str(), it->second.at("test_name_number").c_str(),
          it->second.at("test_name").c_str(), it->second.at("test_result").c_str(),
          it->second.at("test_pattern").c_str(), it->second.at("test_fails").c_str(), it->second.at("test_pin").c_str(),
          it->second.at("test_measured").c_str(), it->second.at("test_minumum").c_str(),
          it->second.at("test_maximum").c_str(), it->second.at("test_measurement_unit").c_str(),
          it->second.at("test_site").c_str(), it->second.at("test_force").c_str(),
          it->second.at("test_force_unit").c_str(), it->second.at("test_channel").c_str(),
          it->second.at("test_key").c_str());
    }
  }
}

void ft17_parser::print_report(
    std::vector<
        std::tuple<int, std::string,
                   std::pair<std::map<std::string, std::string>, std::map<int, std::map<std::string, std::string>>>,
                   std::string, std::string>> &ft17_metadata) {
  int lots_count = ft17_metadata.size();
  int lots_failed = 0;
  int lots_passed = 0;
  std::string failed_lots;

  for (unsigned int i = 0; i < lots_count; i++)
    if (std::get<3>(ft17_metadata[i]) == "FAIL") {

      if (failed_lots.length())
        failed_lots += " ";
      failed_lots += std::to_string(std::get<0>(ft17_metadata[i]));
      lots_failed++;

    } else
      lots_passed++;

  std::printf("FT17 report : \r\n");
  std::printf("Passed : %i;\r\nFailed : %i ( %s );\r\nTotal : %i;\r\n", lots_passed, lots_failed,
              (lots_failed) ? failed_lots.c_str() : "None", lots_count);
}

std::vector<std::string> ft17_parser::ft17_get_blocks(std::string &raw_data) {
  QString data = QString::fromStdString(raw_data);
  std::vector<std::string> result;
  QRegularExpression header_regexp(
      "Программа: [\\p{L} \\p{P}\\p{N}\\p{So}\\p{Sm}]*[\u000d\u000a]+Запуск №[\\p{L} "
      "\\p{P}\\p{N}\\p{So}\\p{Sm}]*[\u000d\u000a]+Уст-во №[\\p{L} \\p{P}\\p{N}\\p{So}\\p{Sm}]*",
      QRegularExpression::UseUnicodePropertiesOption);
  QRegularExpressionMatchIterator header_regexp_it = header_regexp.globalMatch(data);

  QStringList bodies = data.split(header_regexp);
  QStringList headers;

  while (header_regexp_it.hasNext()) {

    QRegularExpressionMatch header_match = header_regexp_it.next();
    headers.append(header_match.captured(0));
  }

  for (unsigned int i = 0; i < headers.size(); i++) {

    result.push_back((headers[i] + bodies[i + 1]).toStdString());
  }

  return result;
}

std::string ft17_parser::ft17_get_header(std::string &block) {
  QString data = QString::fromStdString(block);
  std::string result;
  QRegularExpression header_regexp(
      "Программа:[\\p{L}\\p{Z}\\p{P}\\p{N}\\p{So}\\p{Sm}]*[\u000d\u000a]+Запуск[\\p{L}\\p{Z}\\p{P}\\p{N}\\p{So}\\p{Sm}]"
      "*[\u000d\u000a]+Уст-во[\\p{L}\\p{Z}\\p{P}\\p{N}\\p{So}\\p{Sm}]*",
      QRegularExpression::UseUnicodePropertiesOption);
  QRegularExpressionMatch header_regexp_match = header_regexp.match(data);

  if (header_regexp_match.hasMatch()) {

    result = header_regexp_match.captured(0).toStdString();
  }

  return result;
}

std::string ft17_parser::ft17_get_body(std::string &block) {
  QString data = QString::fromStdString(block);
  std::string result;
  QRegularExpression header_regexp(
      "Программа:[\\p{L}\\p{Z}\\p{P}\\p{N}\\p{So}\\p{Sm}]*[\u000d\u000a]+Запуск[\\p{L}\\p{Z}\\p{P}\\p{N}\\p{So}\\p{Sm}]"
      "*[\u000d\u000a]+Уст-во[\\p{L}\\p{Z}\\p{P}\\p{N}\\p{So}\\p{Sm}]*",
      QRegularExpression::UseUnicodePropertiesOption);
  result = data.split(header_regexp)[1].toStdString();
  return result;
}

std::string ft17_parser::ft17_parse_prog_name(std::string &header) {
  QString data = QString::fromStdString(header);
  std::string result;

  std::function<std::string(std::string &, QRegularExpression &)> parse_substr =
      [](std::string &raw_data, QRegularExpression &re) -> std::string {
    QString data = QString::fromStdString(raw_data);
    std::string result;
    QRegularExpressionMatch match = re.match(data);

    if (match.hasMatch()) {

      result = match.captured(0).toStdString();
    }

    std::stringstream stream(result);

    for (std::string str; stream >> str;)
      result = str;

    return result;
  };

  QRegularExpression prog_name_regexp("Программа: [\\p{Sm}_№\\p{L}\\p{N}]*",
                                      QRegularExpression::UseUnicodePropertiesOption);
  QRegularExpression prog_name_substr_regexp("[ ]+[\\p{Sm}_№\\p{L}\\p{N}]*",
                                             QRegularExpression::UseUnicodePropertiesOption);
  QRegularExpressionMatch prog_name_regexp_match = prog_name_regexp.match(data);

  if (prog_name_regexp_match.hasMatch()) {

    result = prog_name_regexp_match.captured(0).toStdString();
  }

  result = parse_substr(result, prog_name_substr_regexp);
  return result;
}

std::string ft17_parser::ft17_parse_job_name(std::string &header) {
  QString data = QString::fromStdString(header);
  std::string result;

  std::function<std::string(std::string &, QRegularExpression &)> parse_substr =
      [](std::string &raw_data, QRegularExpression &re) -> std::string {
    QString data = QString::fromStdString(raw_data);
    std::string result;
    QRegularExpressionMatch match = re.match(data);

    if (match.hasMatch()) {

      result = match.captured(0).toStdString();
    }

    std::stringstream stream(result);

    for (std::string str; stream >> str;)
      result = str;

    return result;
  };

  QRegularExpression job_name_regexp("Уст-во: [\\p{Sm}_№\\p{L}\\p{N}]*[+-]*",
                                     QRegularExpression::UseUnicodePropertiesOption);
  QRegularExpression job_name_substr_regexp("[ ]+[\\p{Sm}_№\\p{L}\\p{N}]*[+-]*",
                                            QRegularExpression::UseUnicodePropertiesOption);
  QRegularExpressionMatch job_name_regexp_match = job_name_regexp.match(data);

  if (job_name_regexp_match.hasMatch()) {

    result = job_name_regexp_match.captured(0).toStdString();
  }

  result = parse_substr(result, job_name_substr_regexp);
  return result;
}

std::string ft17_parser::ft17_parse_part_name(std::string &header) {
  QString data = QString::fromStdString(header);
  std::string result;

  std::function<std::string(std::string &, QRegularExpression &)> parse_substr =
      [](std::string &raw_data, QRegularExpression &re) -> std::string {
    QString data = QString::fromStdString(raw_data);
    std::string result;
    QRegularExpressionMatch match = re.match(data);

    if (match.hasMatch()) {

      result = match.captured(0).toStdString();
    }

    std::stringstream stream(result);

    for (std::string str; stream >> str;)
      result = str;

    return result;
  };

  QRegularExpression part_name_regexp("Партия: [\\p{Sm}\\p{L}\\p{N}-]*",
                                      QRegularExpression::UseUnicodePropertiesOption);
  QRegularExpression part_name_substr_regexp("[ ]+[\\p{Sm}\\p{L}\\p{N}\\p{P}]*",
                                             QRegularExpression::UseUnicodePropertiesOption);
  QRegularExpressionMatch part_name_regexp_match = part_name_regexp.match(data);

  if (part_name_regexp_match.hasMatch()) {

    result = part_name_regexp_match.captured(0).toStdString();
  }

  result = parse_substr(result, part_name_substr_regexp);
  return result;
}

std::string ft17_parser::ft17_parse_open_datetime(std::string &header) {
  QString data = QString::fromStdString(header);
  std::string result;

  std::function<std::string(std::string &, QRegularExpression &)> parse_substr =
      [](std::string &raw_data, QRegularExpression &re) -> std::string {
    QString data = QString::fromStdString(raw_data);
    std::string result;
    QRegularExpressionMatch match = re.match(data);

    if (match.hasMatch()) {

      result = match.captured(0).toStdString();
    }

    return result;
  };

  QRegularExpression open_datetime_regexp("Открытие: [\\p{N}]+.[\\p{N}]+.[\\p{N}]+ [\\p{N}]+:[\\p{N}]+:[\\p{N}]+",
                                          QRegularExpression::UseUnicodePropertiesOption);
  QRegularExpression open_datetime_substr_regexp("[\\p{N}]+.[\\p{N}]+.[\\p{N}]+ [\\p{N}]+:[\\p{N}]+:[\\p{N}]+",
                                                 QRegularExpression::UseUnicodePropertiesOption);
  QRegularExpressionMatch open_datetime_regexp_match = open_datetime_regexp.match(data);

  if (open_datetime_regexp_match.hasMatch()) {

    result = open_datetime_regexp_match.captured(0).toStdString();
  }

  result = parse_substr(result, open_datetime_substr_regexp);
  return result;
}

std::string ft17_parser::ft17_parse_close_datetime(std::string &header) {
  QString data = QString::fromStdString(header);
  std::string result;

  std::function<std::string(std::string &, QRegularExpression &)> parse_substr =
      [](std::string &raw_data, QRegularExpression &re) -> std::string {
    QString data = QString::fromStdString(raw_data);
    std::string result;
    QRegularExpressionMatch match = re.match(data);

    if (match.hasMatch()) {

      result = match.captured(0).toStdString();
    }

    return result;
  };

  QRegularExpression close_datetime_regexp("Закрытие: [\\p{N}]+.[\\p{N}]+.[\\p{N}]+ [\\p{N}]+:[\\p{N}]+:[\\p{N}]+",
                                           QRegularExpression::UseUnicodePropertiesOption);
  QRegularExpression close_datetime_substr_regexp("[\\p{N}]+.[\\p{N}]+.[\\p{N}]+ [\\p{N}]+:[\\p{N}]+:[\\p{N}]+",
                                                  QRegularExpression::UseUnicodePropertiesOption);
  QRegularExpressionMatch close_datetime_regexp_match = close_datetime_regexp.match(data);

  if (close_datetime_regexp_match.hasMatch()) {

    result = close_datetime_regexp_match.captured(0).toStdString();
  }

  result = parse_substr(result, close_datetime_substr_regexp);
  return result;
}

std::string ft17_parser::ft17_parse_run_number(std::string &header) {
  QString data = QString::fromStdString(header);
  std::string result;

  std::function<std::string(std::string &, QRegularExpression &)> parse_substr =
      [](std::string &raw_data, QRegularExpression &re) -> std::string {
    QString data = QString::fromStdString(raw_data);
    std::string result;
    QRegularExpressionMatch match = re.match(data);

    if (match.hasMatch()) {

      result = match.captured(0).toStdString();
    }

    return result;
  };

  QRegularExpression run_number_regexp("Запуск №[ ]+[\\p{N}]+", QRegularExpression::UseUnicodePropertiesOption);
  QRegularExpression run_number_substr_regexp("[\\p{N}]+", QRegularExpression::UseUnicodePropertiesOption);
  QRegularExpressionMatch run_number_regexp_match = run_number_regexp.match(data);

  if (run_number_regexp_match.hasMatch()) {

    result = run_number_regexp_match.captured(0).toStdString();
  }

  result = parse_substr(result, run_number_substr_regexp);
  return result;
}

std::string ft17_parser::ft17_parse_start_datetime(std::string &header) {
  QString data = QString::fromStdString(header);
  std::string result;

  std::function<std::string(std::string &, QRegularExpression &)> parse_substr =
      [](std::string &raw_data, QRegularExpression &re) -> std::string {
    QString data = QString::fromStdString(raw_data);
    std::string result;
    QRegularExpressionMatch match = re.match(data);

    if (match.hasMatch()) {

      result = match.captured(0).toStdString();
    }

    return result;
  };

  QRegularExpression start_datetime_regexp("Старт: [\\p{N}]+.[\\p{N}]+.[\\p{N}]+ [\\p{N}]+:[\\p{N}]+:[\\p{N}]+",
                                           QRegularExpression::UseUnicodePropertiesOption);
  QRegularExpression start_datetime_substr_regexp("[\\p{N}]+.[\\p{N}]+.[\\p{N}]+ [\\p{N}]+:[\\p{N}]+:[\\p{N}]+",
                                                  QRegularExpression::UseUnicodePropertiesOption);
  QRegularExpressionMatch start_datetime_regexp_match = start_datetime_regexp.match(data);

  if (start_datetime_regexp_match.hasMatch()) {

    result = start_datetime_regexp_match.captured(0).toStdString();
  }

  result = parse_substr(result, start_datetime_substr_regexp);
  return result;
}

std::string ft17_parser::ft17_parse_stop_datetime(std::string &header) {
  QString data = QString::fromStdString(header);
  std::string result;

  std::function<std::string(std::string &, QRegularExpression &)> parse_substr =
      [](std::string &raw_data, QRegularExpression &re) -> std::string {
    QString data = QString::fromStdString(raw_data);
    std::string result;
    QRegularExpressionMatch match = re.match(data);

    if (match.hasMatch()) {

      result = match.captured(0).toStdString();
    }

    return result;
  };

  QRegularExpression stop_datetime_regexp("Стоп: [\\p{N}]+.[\\p{N}]+.[\\p{N}]+ [\\p{N}]+:[\\p{N}]+:[\\p{N}]+",
                                          QRegularExpression::UseUnicodePropertiesOption);
  QRegularExpression stop_datetime_substr_regexp("[\\p{N}]+.[\\p{N}]+.[\\p{N}]+ [\\p{N}]+:[\\p{N}]+:[\\p{N}]+",
                                                 QRegularExpression::UseUnicodePropertiesOption);
  QRegularExpressionMatch stop_datetime_regexp_match = stop_datetime_regexp.match(data);

  if (stop_datetime_regexp_match.hasMatch()) {

    result = stop_datetime_regexp_match.captured(0).toStdString();
  }

  result = parse_substr(result, stop_datetime_substr_regexp);
  return result;
}

std::string ft17_parser::ft17_parse_operator(std::string &header) {
  QString data = QString::fromStdString(header);
  std::string result;

  std::function<std::string(std::string &, QRegularExpression &)> parse_substr =
      [](std::string &raw_data, QRegularExpression &re) -> std::string {
    QString data = QString::fromStdString(raw_data);
    std::string result;
    QRegularExpressionMatch match = re.match(data);

    if (match.hasMatch()) {

      result = match.captured(0).toStdString();
    }

    std::stringstream ss(data.toStdString());

    for (std::string str; ss >> str;)
      result = str;

    return result;
  };

  QRegularExpression operator_regexp("Пользователь: [\\p{L}\\p{N}\\p{P}]*",
                                     QRegularExpression::UseUnicodePropertiesOption);
  QRegularExpression operator_substr_regexp("[\\p{L}\\p{N}\\p{P}]*", QRegularExpression::UseUnicodePropertiesOption);
  QRegularExpressionMatch operator_regexp_match = operator_regexp.match(data);

  if (operator_regexp_match.hasMatch()) {

    result = operator_regexp_match.captured(0).toStdString();
  }

  result = parse_substr(result, operator_substr_regexp);
  return result;
}

int ft17_parser::ft17_parse_lot(std::string &header) {
  QString data = QString::fromStdString(header);
  std::string result;

  std::function<std::string(std::string &, QRegularExpression &)> parse_substr =
      [](std::string &raw_data, QRegularExpression &re) -> std::string {
    QString data = QString::fromStdString(raw_data);
    std::string result;
    QRegularExpressionMatch match = re.match(data);

    if (match.hasMatch()) {

      result = match.captured(0).toStdString();
    }

    return result;
  };

  QRegularExpression lot_regexp("Уст-во № [\\p{N}]+", QRegularExpression::UseUnicodePropertiesOption);
  QRegularExpression lot_substr_regexp("[\\p{N}]+", QRegularExpression::UseUnicodePropertiesOption);
  QRegularExpressionMatch lot_regexp_match = lot_regexp.match(data);

  if (lot_regexp_match.hasMatch()) {

    result = lot_regexp_match.captured(0).toStdString();
  }

  result = parse_substr(result, lot_substr_regexp);
  return std::atoi(result.c_str());
}

std::string ft17_parser::ft17_parse_site(std::string &header) {
  QString data = QString::fromStdString(header);
  std::string result;

  std::function<std::string(std::string &, QRegularExpression &)> parse_substr =
      [](std::string &raw_data, QRegularExpression &re) -> std::string {
    QString data = QString::fromStdString(raw_data);
    std::string result;
    QRegularExpressionMatch match = re.match(data);

    if (match.hasMatch()) {

      result = match.captured(0).toStdString();
    }

    return result;
  };

  QRegularExpression site_regexp("Cайт> [\\p{N}]+", QRegularExpression::UseUnicodePropertiesOption);
  QRegularExpression site_substr_regexp("[\\p{N}]+", QRegularExpression::UseUnicodePropertiesOption);
  QRegularExpressionMatch site_regexp_match = site_regexp.match(data);

  if (site_regexp_match.hasMatch()) {

    result = site_regexp_match.captured(0).toStdString();
  }

  result = parse_substr(result, site_substr_regexp);
  return result;
}

std::string ft17_parser::ft17_parse_result(std::string &header) {
  QString data = QString::fromStdString(header);
  std::string result;

  std::function<std::string(std::string &, QRegularExpression &)> parse_substr =
      [](std::string &raw_data, QRegularExpression &re) -> std::string {
    QString data = QString::fromStdString(raw_data);
    std::string result;
    QRegularExpressionMatch match = re.match(data);

    if (match.hasMatch()) {

      result = match.captured(0).toStdString();
    }

    std::stringstream ss(data.toStdString());

    for (std::string str; ss >> str;)
      result = str;

    return result;
  };

  QRegularExpression result_regexp("Результат: [\\p{L}]+", QRegularExpression::UseUnicodePropertiesOption);
  QRegularExpression result_substr_regexp("[\\p{L}]+", QRegularExpression::UseUnicodePropertiesOption);
  QRegularExpressionMatch result_regexp_match = result_regexp.match(data);

  if (result_regexp_match.hasMatch()) {

    result = result_regexp_match.captured(0).toStdString();
  }

  result = parse_substr(result, result_substr_regexp);
  return result;
}

std::string ft17_parser::ft17_parse_category(std::string &header) {
  QString data = QString::fromStdString(header);
  std::string result;

  std::function<std::string(std::string &, QRegularExpression &)> parse_substr =
      [](std::string &raw_data, QRegularExpression &re) -> std::string {
    QString data = QString::fromStdString(raw_data);
    std::string result;
    QRegularExpressionMatch match = re.match(data);

    if (match.hasMatch()) {

      result = match.captured(0).toStdString();
    }

    return result;
  };

  QRegularExpression category_regexp("Категория: [\\p{N}]+", QRegularExpression::UseUnicodePropertiesOption);
  QRegularExpression category_substr_regexp("[\\p{N}]+", QRegularExpression::UseUnicodePropertiesOption);
  QRegularExpressionMatch category_regexp_match = category_regexp.match(data);

  if (category_regexp_match.hasMatch()) {

    result = category_regexp_match.captured(0).toStdString();
  }

  result = parse_substr(result, category_substr_regexp);
  return result;
}

std::map<int, std::map<std::string, std::string>> ft17_parser::ft17_get_tests(std::string &body) {
  std::function<std::string(std::string &, QRegularExpression &, QRegularExpression &)> get_match =
      [](std::string &raw_data, QRegularExpression &pattern, QRegularExpression &sub_pattern) -> std::string {
    std::string result;
    QString data = QString::fromStdString(raw_data);
    QRegularExpressionMatch pattern_match = pattern.match(data);

    if (pattern_match.hasMatch())
      result = pattern_match.captured(0).toStdString();

    std::stringstream ss(result);

    for (std::string str; ss >> str;)
      result = str;

    QRegularExpressionMatch sub_pattern_match = sub_pattern.match(QString::fromStdString(result));

    if (sub_pattern_match.hasMatch())
      result = sub_pattern_match.captured(0).toStdString();

    return result;
  };

  std::map<int, std::map<std::string, std::string>> result;
  std::vector<std::string> tests;
  QString data = QString::fromStdString(body);
  QRegularExpression test_regexp(
      "([ ]*[\\p{N}]+\\.[ ]+[\\p{N}]+:[\\p{N}]+:[\\p{N}]+\\.[\\p{N}]+[ ]+Тест: [\\p{N}]+\\.[ ]+[\\p{P}\\p{L}\\p{N}]+[ "
      "]+Статус: [\\p{L}]+[ ]+Конт: [\\p{L}\\p{N}\\p{P}]+[ ]+Измер: [\\p{N}\\p{P}\\p{Sm}]+ [\\p{L}]+[ ]+Мин.: "
      "[\\p{N}\\p{P}\\p{Sm}]+ [\\p{L}]+[ ]+Макс: [\\p{N}\\p{P}\\p{Sm}]+ [\\p{L}]+[ ]+Сайт:[\\p{N}]+; "
      "[\\p{Lu}]=[\\p{N}\\p{P}\\p{Sm}]+ [\\p{L}]+; Кан:[\\p{N}]\\.[\\p{N}]\\.[\\p{N}]; к\\.[\\p{N}]+;)|"
      "([ ]*[\\p{N}]+\\.[ ]+[\\p{N}]+:[\\p{N}]+:[\\p{N}]+\\.[\\p{N}]+[ ]+Тест: [\\p{N}]+\\.[ ]+[\\p{P}\\p{L}\\p{N}]+[ "
      "]+Статус: [\\p{L}]+[ ]+Патн: [\\p{L}\\p{N}\\p{P}\\p{Z}\\p{Sm}\\p{So}]+)");
  QRegularExpressionMatchIterator test_regexp_it = test_regexp.globalMatch(data);

  std::string tests_header(
      "test_number test_time test_name_number test_name test_result test_pattern test_fails test_pin test_measured "
      "test_minumum test_maximum test_measurement_unit test_site test_force test_force_unit test_channel test_key");
  std::map<std::string, std::tuple<QRegularExpression, QRegularExpression>> test_regex_map = {
      {"test_time",
       {QRegularExpression("[ ]+[\\p{N}]+:[\\p{N}]+:[\\p{N}]+\\.[\\p{N}]+"),
        QRegularExpression("[\\p{N}]+:[\\p{N}]+:[\\p{N}]+\\.[\\p{N}]+")}},
      {"test_name_number", {QRegularExpression("Тест: [\\p{N}]+"), QRegularExpression("[\\p{N}]+")}},
      {"test_name",
       {QRegularExpression("Тест: [\\p{N}]+\\.[ ]+[\\p{P}\\p{L}\\p{N}]+"),
        QRegularExpression("[\\p{P}\\p{L}\\p{N}]+")}},
      {"test_result", {QRegularExpression("Статус: [\\p{L}]+"), QRegularExpression("[\\p{L}]+")}},
      {"test_pattern",
       {QRegularExpression("Патн: [\\p{N}\\p{L}\\p{P}_]+"), QRegularExpression("[\\p{N}\\p{L}\\p{P}_]+")}},
      {"test_fails", {QRegularExpression("Сбоев: [\\p{N}]+"), QRegularExpression("[\\p{N}]+")}},
      {"test_pin", {QRegularExpression("Конт: [\\p{L}\\p{N}\\p{P}]+"), QRegularExpression("[\\p{L}\\p{N}\\p{P}]+")}},
      {"test_measured",
       {QRegularExpression("Измер: [\\p{N}\\p{P}\\p{Sm}]+"), QRegularExpression("[\\p{N}\\p{P}\\p{Sm}]+")}},
      {"test_minumum",
       {QRegularExpression("Мин.: [\\p{N}\\p{P}\\p{Sm}]+"), QRegularExpression("[\\p{N}\\p{P}\\p{Sm}]+")}},
      {"test_maximum",
       {QRegularExpression("Макс: [\\p{N}\\p{P}\\p{Sm}]+"), QRegularExpression("[\\p{N}\\p{P}\\p{Sm}]+")}},
      {"test_measurement_unit",
       {QRegularExpression("Измер: [\\p{N}\\p{P}\\p{Sm}]+ [\\p{L}]+"), QRegularExpression("[\\p{L}]+")}},
      {"test_site", {QRegularExpression("Сайт:[\\p{N}]+"), QRegularExpression("[\\p{N}]+")}},
      {"test_force", {QRegularExpression("[\\p{Lu}]=[\\p{N}\\p{P}\\p{Sm}]+"), QRegularExpression("[\\p{N}\\p{P}]+")}},
      {"test_force_unit",
       {QRegularExpression("[\\p{Lu}]=[\\p{N}\\p{P}\\p{Sm}]+ [\\p{L}]+"), QRegularExpression("[\\p{L}]+")}},
      {"test_channel",
       {QRegularExpression("Кан:[\\p{N}]\\.[\\p{N}]\\.[\\p{N}]"),
        QRegularExpression("[\\p{N}]\\.[\\p{N}]\\.[\\p{N}]")}},
      {"test_key", {QRegularExpression("к\\.[\\p{N}]+"), QRegularExpression("[\\p{N}]+")}}};
  QStringList tests_header_list = QString::fromStdString(tests_header).split(" ");
  QRegularExpression test_time_regexp("");

  while (test_regexp_it.hasNext()) {

    QRegularExpressionMatch match = test_regexp_it.next();
    tests.push_back(match.captured(0).toStdString());
  }

  for (std::string test : tests) {

    std::map<std::string, std::string> test_meta;
    std::stringstream ss(test);
    std::string test_number;
    ss >> test_number;

    QRegularExpression test_number_regexp("[\\p{N}]+");
    QRegularExpressionMatch test_number_regexp_match = test_number_regexp.match(QString::fromStdString(test_number));

    if (test_number_regexp_match.hasMatch())
      test_number = test_number_regexp_match.captured(0).toStdString();

    for (unsigned int i = 1; i < tests_header_list.size(); i++)
      test_meta.insert(
          std::make_pair(tests_header_list[i].toStdString(),
                         get_match(test, std::get<0>(test_regex_map.at(tests_header_list[i].toStdString())),
                                   std::get<1>(test_regex_map.at(tests_header_list[i].toStdString())))));

    for (unsigned int i = 0; i < test_meta.at("test_measured").length(); i++)
      if (test_meta.at("test_measured")[i] == ',')
        test_meta.at("test_measured")[i] = '.';

    result.insert(std::make_pair(std::atoi(test_number.c_str()), test_meta));
  }

  return result;
}

std::tuple<std::string, std::string>
ft17_parser::ft17_check_tests(std::map<int, std::map<std::string, std::string>> &tests_meta) {
  std::tuple<std::string, std::string> result;

  for (std::map<int, std::map<std::string, std::string>>::iterator it = tests_meta.begin(); it != tests_meta.end();
       it++) {

    if (it->second.at("test_result") == "Годен")
      result = {"PASS", "OK"};
    else if (it->second.at("test_result") == "Брак") {

      result = {"FAIL", it->second.at("test_name")};
      break;
    }
  }

  return result;
}
