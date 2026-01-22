// Loader.cpp : 
//
#include"Loader.h"


// ---------------------------------------------------------------
// 1. Структура, хранящая строку и её метаданные
// ---------------------------------------------------------------
struct LineInfo
{
	std::string text;          // сама строка без '\n'
	std::size_t globalIndex;   // номер строки в файле (начинается с 1)
	std::string sourceFile;    // имя файла

	friend std::ostream& operator<<(std::ostream& os, const LineInfo& li)
	{
		return os << '[' << std::filesystem::path(li.sourceFile).filename() << "] "
			<< li.globalIndex << ": " << li.text;
	}
};

// ---------------------------------------------------------------
// 2. Чтение N строк и формирование LineInfo
// ---------------------------------------------------------------
std::vector<LineInfo> read_n_lines(std::istream& is,
	std::size_t N,
	std::size_t startIdx,
	const std::string& fileName)
{
	std::vector<LineInfo> out;
	out.reserve(N);

	std::string line;
	for (std::size_t i = 0; i < N && std::getline(is, line); ++i)
	{
		out.emplace_back(LineInfo{
			std::move(line),          // текст
			startIdx + i,             // глобальный номер строки
			fileName                  // имя файла
			});
	}
	return out;
}

// ---------------------------------------------------------------
// 3. Чтение файла «порциями» по 50 строк
// ---------------------------------------------------------------
std::vector<std::vector<LineInfo>> read_file_by_50(const std::string& filename)
{
	std::ifstream file(filename, std::ios::binary);
	if (!file)
		throw std::runtime_error("Cannot open file: " + filename);

	constexpr std::size_t batchSize = 50;
	std::vector<std::vector<LineInfo>> allBatches;

	std::size_t nextGlobalIdx = 1;               // первая строка имеет номер 1
	while (!file.eof())
	{
		auto batch = read_n_lines(file,
			batchSize,
			nextGlobalIdx,
			filename);
		if (batch.empty())
			break;                               // конец файла

		nextGlobalIdx += batch.size();           // подготовим номер первой строки следующей партии
		allBatches.emplace_back(std::move(batch));
	}
	return allBatches;
}

namespace fs = std::filesystem;

bool Loader(const std::string FName, const nlohmann::json& json_config)
{
	try
	{
		////------------------------------ Debugger ---------------------------------------
		std::cout << std::endl << std::endl << "Processing ... " << std::endl << std::endl;
		//---------------------------------------------------------------------------------

		HTTP http_class;
		//if (http_gpt_class.CheckChatGptRequest())
		//{
		while (true)
		{
			if (fs::is_regular_file(std::filesystem::path(FName)))
			{
				if (FName.find(".txt") != std::string::npos)
				{
					//Log.WriteStartLog();

					std::ifstream file(std::filesystem::path(FName), std::ios::binary);
					if (!file.is_open())
					{
						std::cerr << "Error: File \"" + FName + "\" cannot be opened!" << std::endl;
						std::cout << "Operation failed!" << std::endl << std::endl;
						return false;
					}

					file.close();

					auto batches = read_file_by_50(std::filesystem::path(FName).string());

					if (true)
					{
						std::cout << "File " << std::filesystem::path(FName).filename() << " was split into "
							<< batches.size() << " batch(es) (max 50 lines each).\n\n";
					}

					for (std::size_t batchIdx = 0; batchIdx < batches.size(); ++batchIdx)
					{
						const auto& batch = batches[batchIdx];
						
						// ---- 1) name ----------------------------------------------------
						std::string name = fs::path(batch.front().sourceFile).stem().string();

						// ---- 2) rows ----------------------------------------------------
						// Формируем строку диапазона «начало‑конец», где начало = первой
						// строки партии, конец = последней строки партии.
						std::size_t firstRow = batch.front().globalIndex;
						std::size_t lastRow = batch.back().globalIndex;
						std::string rows = std::to_string(firstRow) + "-" + std::to_string(lastRow);

						// ---- 3) text ----------------------------------------------------
						// Склеиваем все строки партии в один блок, разделяя их '\n'.
                        // В конце строки не добавляем лишний перевод строки.
						std::string text;
                        text.reserve(batch.size() * 80); // приблизительная оценка
						for (std::size_t i = 0; i < batch.size(); ++i)
						{
							text += batch[i].text;
                            if (i + 1 != batch.size())   // добавляем \n только между строками
								text += '\n';
						}

						// ---- 4) Формируем JSON‑объект -----------------------------------
						nlohmann::json batchJson;
						batchJson["name"] = std::move(name);
						batchJson["rows"] = std::move(rows);
						batchJson["text"] = std::move(text);

						// -------------------- Вывод в консоль (для отладки) -------------
						if (true)
						{
							std::cout << "=== Batch #" << batchIdx + 1
								<< " (rows " << batchJson["rows"] << ") ===\n\n";
						}
						if (false)
						{
							std::cout << "=== Batch #" << batchIdx + 1
								<< " (rows " << batchJson["rows"] << ") ===\n"
								<< batchJson.dump(4) << "\n\n";
						}
                        //  реальные переносы строк
						if (false)
						{
							std::cout << "TEXT (real new-lines):\n"
								<< batchJson["text"].get<std::string>() << "\n\n";
						}
						
						// -------------------- Отправляем в S3 -------------------------
						http_class.S3_PutJsonObject(batchJson.dump());


						std::this_thread::sleep_for(std::chrono::seconds{ 1 }); // пауза 1 секунда
					}

					//Metr.UpdateNumFiles();
					//Metr.UpdateTotalTime();
					//
					//Log.WriteEndLog();
				}
				else
				{
					std::cerr << "Error: File \"" + FName + "\" does not exist or is not valid!" << std::endl;
					std::cout << "Operation failed!" << std::endl << std::endl;
					return false;
				}
			}
			else
			{
				std::cerr << "Error: File \"" + FName + "\" does not exist or is not valid!" << std::endl;
				std::cout << "Operation failed!" << std::endl << std::endl;
				return false;
			}
			//}

			//Metr.MainMetrics();
			//}
		}
			std::cout << "Operation completed successfully!" << std::endl << std::endl;
			//return "";
			return true;
		//}
		//}
		//else
		//{
		//	std::cerr << "Error: The neural model is unavailable!" << std::endl;
		//	std::cout << "Operation failed!" << std::endl << std::endl;
		//	//return "Error: The neural model is unavailable!";
		//	return false;
		//}
	}
	catch (const ConfigError& e)   //  конфигурационные ошибки
	{
		std::cerr << "[CONFIG ERROR] " << e.what() << '\n';
		return false;
	}
	catch (const std::exception& e) // любые другие runtime‑исключения
	{
		std::cerr << "[RUNTIME ERROR] " << e.what() << '\n';
		return false;
	}
	catch (...)
	{
		std::cerr << "Unknown error in inner function!" << std::endl;
		std::cout << "Operation failed!" << std::endl << std::endl;
		//return "Unknown error in inner function!";
		return false;
	}

	return 0;
}


bool onApiInput(const char* FName, const char* config)
{
	if (FName != nullptr)
	{
		const std::string dirname_in_str(FName);

		if (!dirname_in_str.empty())
		{
			try
			{
				// конфиг не нуждается в проверке

				bool flag = Loader(dirname_in_str, NULL);

				return flag;
			}
			catch (...)
			{
				std::cerr << "Unknown error in external function!" << std::endl;
				return false;
			}
		}
		else
		{
			std::cerr << "No dir name!" << std::endl;
			return false;
		}
	}
	else
	{
		std::cerr << "No dir name!" << std::endl;
		return false;
	}
}
