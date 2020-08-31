#include "rclcpp/rclcpp.hpp"
#include <cstdio>

#include "rosbag2_compression/compression_options.hpp"
#include "rosbag2_compression/sequential_compression_reader.hpp"
#include "rosbag2_compression/sequential_compression_writer.hpp"
#include "rosbag2_cpp/info.hpp"
#include "rosbag2_transport/rosbag2_transport.hpp"
#include <rosbag2_cpp/reader.hpp>
#include <rosbag2_cpp/writer.hpp>

int main(int argc, char ** argv)
{
  (void) argc;
  (void) argv;

  rosbag2_transport::PlayOptions play_options{};
  rosbag2_transport::StorageOptions storage_options{};

  const char * uri = "sample_rosbag";
  const char *storage_id = "sqlite3";
  const char * node_prefix = "test_";
  size_t read_ahead_queue_size = 1000;
  float rate = 1;
  bool loop = false;

  storage_options.uri = std::string(uri);
  storage_options.storage_id = std::string(storage_id);

  play_options.node_prefix = std::string(node_prefix);
  play_options.read_ahead_queue_size = read_ahead_queue_size;
  play_options.rate = rate;
  play_options.loop = loop;

  rosbag2_storage::MetadataIo metadata_io{};
  rosbag2_storage::BagMetadata metadata{};
  // Specify defaults
  auto info = std::make_shared<rosbag2_cpp::Info>();
  std::shared_ptr<rosbag2_cpp::Reader> reader;
  auto writer = std::make_shared<rosbag2_cpp::Writer>(
    std::make_unique<rosbag2_cpp::writers::SequentialWriter>());
  // Change reader based on metadata options
  if (metadata_io.metadata_file_exists(storage_options.uri)) {
    metadata = metadata_io.read_metadata(storage_options.uri);
    if (metadata.compression_format == "zstd") {
      reader = std::make_shared<rosbag2_cpp::Reader>(
        std::make_unique<rosbag2_compression::SequentialCompressionReader>());
    } else {
      reader = std::make_shared<rosbag2_cpp::Reader>(
        std::make_unique<rosbag2_cpp::readers::SequentialReader>());
    }
  } else {
    reader = std::make_shared<rosbag2_cpp::Reader>(
      std::make_unique<rosbag2_cpp::readers::SequentialReader>());
  }

  rosbag2_transport::Rosbag2Transport transport(reader, writer, info);
  transport.init();
  transport.play(storage_options, play_options);
  transport.shutdown();

  return 0;
}
