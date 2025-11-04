#include <filesystem>
#include <string>

#include <gtest/gtest.h>

#include <objectstore.hpp>
#include <scopeguard.hpp>

TEST(StoredFile, Basics) {
  auto resource = std::pmr::get_default_resource();
  auto file = objectstore::StoredFile{resource, "objectstore_test_basic.dat"};
  file.destroy();
  auto _ = common::MakeScopeGuard([&file] {
    file.destroy();
    ASSERT_FALSE(file.exists());
  });

  EXPECT_FALSE(std::filesystem::exists(file.path()));
  EXPECT_EQ(std::filesystem::exists(file.path()), file.exists());
  EXPECT_FALSE(file.is_open());

  file.open();
  EXPECT_TRUE(file.is_open());
  EXPECT_TRUE(std::filesystem::exists(file.path()));
  EXPECT_EQ(std::filesystem::exists(file.path()), file.exists());

  {
    auto size_pair = file.size();
    EXPECT_EQ(size_pair.first, 0u);
    EXPECT_FALSE(size_pair.second);
  }

  file.close();
  EXPECT_FALSE(file.is_open());
  EXPECT_TRUE(std::filesystem::exists(file.path()));
  EXPECT_EQ(std::filesystem::exists(file.path()), file.exists());

  file.destroy();
  EXPECT_FALSE(std::filesystem::exists(file.path()));
  EXPECT_EQ(std::filesystem::exists(file.path()), file.exists());
}

TEST(StoredFile, ScopeDestroy) {
  auto resource = std::pmr::get_default_resource();
  auto file = objectstore::StoredFile{resource, "objectstore_test_scope.dat"};
  ASSERT_TRUE(!file.exists());

  {
    auto _ = common::MakeScopeGuard([&file] {
      file.destroy();
      ASSERT_FALSE(file.exists());
    });

    {
      auto stream = file.stream();
      stream->write("Temporary data", 14);
      stream->flush();
    }

    ASSERT_TRUE(file.exists());
  }

  ASSERT_FALSE(file.exists());
}

TEST(StoredFile, WriteRead) {
  auto resource = std::pmr::get_default_resource();
  auto file = objectstore::StoredFile{resource, "objectstore_test_write.dat"};
  file.destroy();
  auto _ = common::MakeScopeGuard([&file] {
    file.destroy();
    EXPECT_FALSE(file.exists());
  });

  std::string const data = "Hello, ObjectStore!";

  {
    auto stream = file.stream();
    stream->write(data.data(), data.size());
    stream->flush();
  }

  {
    auto size_pair = file.size();
    EXPECT_EQ(size_pair.first, data.size());
    EXPECT_FALSE(size_pair.second);
  }

  {
    auto const stream = file.stream();
    stream->seekg(0);
    std::string read_data(data.size(), '\0');
    stream->read(read_data.data(), read_data.size());
    EXPECT_EQ(read_data, data);
  }

  file.close();
  EXPECT_FALSE(file.is_open());

  {
    auto *stream = file.stream();
    EXPECT_TRUE(file.is_open());
    EXPECT_TRUE(stream->good());
    EXPECT_EQ(stream->tellg(), 0);
    auto size_res = file.size();
    ASSERT_FALSE(size_res.second);
    EXPECT_EQ(size_res.first, data.size());
    std::string read_data(size_res.first, '\0');
    stream->read(read_data.data(), read_data.size());
    EXPECT_EQ(read_data, data);
  }
}

TEST(StoredFolder, Basics) {
  auto resource = std::pmr::get_default_resource();
  auto folder = objectstore::StoredFolder{resource, "objectstore_test_folder"};
  auto _ = common::MakeScopeGuard([&folder] {
    folder.clear();
    auto path = folder.path();
    EXPECT_TRUE(std::filesystem::exists(path));
    size_t num_files = 0;
    for (auto const &dir_iter : std::filesystem::directory_iterator(path)) {
      ++num_files;
    }
    EXPECT_EQ(num_files, 0);
    std::filesystem::remove_all("objectstore_test_folder");
  });

  auto id1 = folder.add();
  auto id2 = folder.add();

  std::string const data_file_1 = "Data for file 1";
  std::string const data_file_2 = "Data for file 2, which contains more words";

  {
    auto stream1 = folder.get(id1);
    stream1->write(data_file_1.data(), data_file_1.size());
    stream1->flush();
  }

  {
    auto stream2 = folder.get(id2);
    stream2->write(data_file_2.data(), data_file_2.size());
    stream2->flush();
  }

  {
    auto stream1 = folder.get(id1);
    std::string data1(folder.size(id1).first, '\0');
    stream1->read(data1.data(), data1.size());
    EXPECT_EQ(data1, data_file_1);
  }

  {
    auto stream2 = folder.get(id2);
    std::string data2(folder.size(id2).first, '\0');
    stream2->read(data2.data(), data2.size());
    EXPECT_EQ(data2, data_file_2);
  }
}
