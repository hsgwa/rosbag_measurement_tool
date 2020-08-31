#include "rclcpp/rclcpp.hpp"
#include <cstdio>

#include "rosbag2_compression/compression_options.hpp"
#include "rosbag2_compression/sequential_compression_reader.hpp"
#include "rosbag2_compression/sequential_compression_writer.hpp"
#include "rosbag2_cpp/info.hpp"
#include "rosbag2_transport/rosbag2_transport.hpp"
#include <rosbag2_cpp/reader.hpp>
#include <rosbag2_cpp/writer.hpp>
#include <sys/stat.h>

int main(int argc, char ** argv)
{
  (void) argc;
  (void) argv;

  rosbag2_transport::StorageOptions storage_options{};
  rosbag2_transport::RecordOptions record_options{};

  const char * uri = "rosbag2_sample";
  const char * storage_id = "sqlite3";
  const char * serilization_format = "";
  const char * node_prefix = "sample_";
  const char * compression_mode = "none";
  const char * compression_format = "";
  bool all = true;
  bool no_discovery = false;
  uint64_t polling_interval_ms = 100;
  unsigned long long max_bagfile_size = 0;  // NOLINT
  unsigned long long max_bagfile_duration = 0;  // NOLINT
  uint64_t max_cache_size = 0u;
  bool include_hidden_topics = false;

  storage_options.uri = std::string(uri);
  storage_options.storage_id = std::string(storage_id);
  storage_options.max_bagfile_size = (uint64_t) max_bagfile_size;
  storage_options.max_bagfile_duration = static_cast<uint64_t>(max_bagfile_duration);
  storage_options.max_cache_size = max_cache_size;
  record_options.all = all;
  record_options.is_discovery_disabled = no_discovery;
  record_options.topic_polling_interval = std::chrono::milliseconds(polling_interval_ms);
  record_options.node_prefix = std::string(node_prefix);
  record_options.compression_mode = std::string(compression_mode);
  record_options.compression_format = compression_format;
  record_options.include_hidden_topics = include_hidden_topics;
  struct stat st;
  if (stat(uri, &st) ==0 ) {
    std::cout << uri << " already exist." << std::endl;
    return 1;
  }
  if (mkdir(uri, 0777) != 0) {
    std::cout << "failued to create " << uri << std::endl;
    return 1;
  }

  rosbag2_compression::CompressionOptions compression_options{
    record_options.compression_format,
    rosbag2_compression::compression_mode_from_string(record_options.compression_mode)
  };

  record_options.rmw_serialization_format = std::string(serilization_format).empty() ?
    rmw_get_serialization_format() :
    serilization_format;

  // Specify defaults
  auto info = std::make_shared<rosbag2_cpp::Info>();
  auto reader = std::make_shared<rosbag2_cpp::Reader>(
    std::make_unique<rosbag2_cpp::readers::SequentialReader>());
  std::shared_ptr<rosbag2_cpp::Writer> writer;
  // Change writer based on recording options
  if (record_options.compression_format == "zstd") {
    writer = std::make_shared<rosbag2_cpp::Writer>(
      std::make_unique<rosbag2_compression::SequentialCompressionWriter>(compression_options));
  } else {
    writer = std::make_shared<rosbag2_cpp::Writer>(
      std::make_unique<rosbag2_cpp::writers::SequentialWriter>());
  }

  rosbag2_transport::Rosbag2Transport transport(reader, writer, info);
  transport.init();
  transport.record(storage_options, record_options);
  transport.shutdown();

  return 0;
}
