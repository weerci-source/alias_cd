#include <algorithm>   
#include <gtest/gtest.h>
#include <fstream>
#include <sstream>
#include <thread>
#include <vector>
#include <chrono>
#include <filesystem> // C++17 или использовать boost

namespace fs = std::filesystem;

// Вспомогательная функция для чтения файла
std::string ReadFileContent(const std::string& path) {
	std::ifstream file(path);
	if (!file.is_open()) return "";
	std::stringstream buffer;
	buffer << file.rdbuf();
	return buffer.str();
}

// Генерация временного файла
std::string GetTempFileName() {
	static int counter = 0;
	return fs::temp_directory_path().string() + "/test_log_" + std::to_string(counter++) + ".log";
}

// Тестовый класс с настройкой и очисткой
class LoggerTest : public ::testing::Test {
protected:
	static void SetUpTestSuite() {
		InitLocale();
	}

	void TearDown() override {
		Logger::Instance().Close();   // закрываем логгер
		if (!currentLogFile_.empty() && fs::exists(currentLogFile_)) {
			fs::remove(currentLogFile_);
		}
		currentLogFile_.clear();
	}

	// Создать новый лог-файл и инициализировать логгер
	bool InitNewLogger(const std::string& path = "") {
		currentLogFile_ = path.empty() ? GetTempFileName() : path;
		return Logger::Instance().Init(currentLogFile_);
	}

	std::string currentLogFile_;
};

// 1. Тест успешной инициализации
TEST_F(LoggerTest, InitSuccess) {
	ASSERT_TRUE(InitNewLogger());
	// Проверим, что файл создан
	EXPECT_TRUE(fs::exists(currentLogFile_));
}

// 2. Повторная инициализация не переоткрывает файл
TEST_F(LoggerTest, InitTwiceDoesNotReopen) {
	std::string path1 = GetTempFileName();
	std::string path2 = GetTempFileName();

	EXPECT_TRUE(Logger::Instance().Init(path1));
	// После успешного открытия, второй Init с другим путём должен вернуть true,
	// но файл path2 не должен быть создан.
	EXPECT_TRUE(Logger::Instance().Init(path2));
	EXPECT_TRUE(fs::exists(path1));
	EXPECT_FALSE(fs::exists(path2)); // path2 не должен создаться

	// Почистим
	fs::remove(path1);
}

// 3. Ошибка инициализации (недоступный каталог)
TEST_F(LoggerTest, InitFailure) {
	std::string badPath = "/nonexistent/dir/log.log";
	EXPECT_FALSE(Logger::Instance().Init(badPath));
	// Файл не должен быть создан
	EXPECT_FALSE(fs::exists(badPath));
}

// 4. Логирование строк (файл создан)
TEST_F(LoggerTest, LogString) {
	ASSERT_TRUE(InitNewLogger());
	Logger::Instance().Log("Hello, world!");
	Logger::Instance().Log("Second line");

	// Дадим время на запись (flush уже есть, но на всякий)
	std::this_thread::sleep_for(std::chrono::milliseconds(10));

	std::string content = ReadFileContent(currentLogFile_);
	EXPECT_FALSE(content.empty());
	// Проверим, что обе строки присутствуют (вместе с временной меткой)
	EXPECT_NE(content.find("Hello, world!"), std::string::npos);
	EXPECT_NE(content.find("Second line"), std::string::npos);
}

// 5. Логирование широких строк (если есть тестовые данные)
TEST_F(LoggerTest, LogWideString) {
	ASSERT_TRUE(InitNewLogger());
	std::wstring wideMsg = L"Тест Юникод 😊";
	Logger::Instance().Log(wideMsg);

	std::string content = ReadFileContent(currentLogFile_);
	EXPECT_NE(content.find("Тест Юникод"), std::string::npos); // UTF-8
	// Проверим наличие смайлика (байтовая последовательность может быть)
	// Можно проверить, что строка содержит символы.
}

// 6. Потокобезопасность: запись из нескольких потоков
TEST_F(LoggerTest, MultithreadedLogging) {
	ASSERT_TRUE(InitNewLogger());
	const int numThreads = 10;
	const int messagesPerThread = 100;
	std::vector<std::thread> threads;
	threads.reserve(numThreads);

	for (int i = 0; i < numThreads; ++i) {
		threads.emplace_back([i, messagesPerThread]() {
			for (int j = 0; j < messagesPerThread; ++j) {
				Logger::Instance().Log("Thread " + std::to_string(i) + " msg " + std::to_string(j));
			}
			});
	}

	for (auto& t : threads) t.join();

	// Проверим, что все строки присутствуют (могут быть перемешаны, но все должны быть)
	std::string content = ReadFileContent(currentLogFile_);
	for (int i = 0; i < numThreads; ++i) {
		// Ищем хотя бы одно сообщение от каждого потока
		EXPECT_NE(content.find("Thread " + std::to_string(i)), std::string::npos);
	}
	// Общее количество строк должно быть numThreads * messagesPerThread
	// (учитывая, что каждая запись — одна строка)
	int lines = std::count(content.begin(), content.end(), '\n');
	EXPECT_EQ(lines, numThreads * messagesPerThread);
}

// 7. Тест: когда файл не открыт, сообщение уходит в std::cerr
TEST_F(LoggerTest, LogWhenFileNotOpen) {
    // 1. Убедимся, что логгер закрыт (если был открыт из предыдущего теста)
    Logger::Instance().Close();

    // 2. Перехватываем std::cerr в строковый поток
    std::stringstream buffer;
    std::streambuf* old_cerr_buf = std::cerr.rdbuf(buffer.rdbuf());

    // 3. Логируем сообщение
    const std::string test_msg = "Message when file is closed";
    Logger::Instance().Log(test_msg);

    // 4. Восстанавливаем оригинальный буфер std::cerr
    std::cerr.rdbuf(old_cerr_buf);

    // 5. Проверяем, что сообщение попало в std::cerr
    std::string output = buffer.str();
    EXPECT_NE(output.find(test_msg), std::string::npos)
        << "Expected message not found in cerr output";
    
    // 6. Проверяем, что есть пометка [LOGGER] (если вы её добавили в Write)
    //    Если у вас в Write() нет такой пометки, можете убрать эту проверку
    EXPECT_NE(output.find("[LOGGER]"), std::string::npos)
        << "Expected '[LOGGER]' prefix not found in cerr output";
}

// 8. Проверка временной метки (приблизительно)
TEST_F(LoggerTest, TimestampFormat) {
	ASSERT_TRUE(InitNewLogger());
	Logger::Instance().Log("test");

	std::string content = ReadFileContent(currentLogFile_);
	// Ожидаем, что строка начинается с [YYYY-MM-DD HH:MM:SS]
	// Можно проверить регулярным выражением, но упростим:
	EXPECT_TRUE(content.find('[') != std::string::npos);
	EXPECT_TRUE(content.find(']') != std::string::npos);
	// Проверим, что год 20xx
	size_t yearPos = content.find(']');
	if (yearPos != std::string::npos && yearPos >= 5) {
		std::string year = content.substr(1, 4); // первая цифра года
		EXPECT_TRUE(year >= "2020" && year <= "2099");
	}
}