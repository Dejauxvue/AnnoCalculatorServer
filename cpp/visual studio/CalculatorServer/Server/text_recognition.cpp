#include "text_recognition.hpp"

#include <algorithm>
#include <regex>

std::shared_ptr<tesseract::TessBaseAPI> text_recognition::ocr_(nullptr);

std::vector<std::pair<std::string, cv::Rect>> text_recognition::detect_words(const cv::InputArray &in )
{
	

	cv::Mat input = in.getMat();

	ocr()->SetPageSegMode(tesseract::PSM_AUTO);

	// Set image data
	ocr()->SetImage(input.data, input.cols, input.rows, 4, input.step);

	ocr()->Recognize(0);
	tesseract::ResultIterator* ri = ocr()->GetIterator();
	tesseract::PageIteratorLevel level = tesseract::RIL_WORD;

	std::vector<std::pair<std::string, cv::Rect>> ret;
	
	if (ri != 0) {
		ret.reserve(10);
		do {
			const char* word = ri->GetUTF8Text(level);
			float conf = ri->Confidence(level);
			int x1, y1, x2, y2;
			ri->BoundingBox(level, &x1, &y1, &x2, &y2);
			printf("word: '%s';  \tconf: %.2f; BoundingBox: %d,%d,%d,%d;\n",
				word, conf, x1, y1, x2, y2);
			std::string word_s = word ? std::string(word) : std::string();
			cv::Rect aa_bb(cv::Point(x1, y1), cv::Point(x2, y2));
			ret.push_back(std::make_pair(word_s, aa_bb));
			delete[] word;
		} while (ri->Next(level));
	}

	return ret;
}

std::map<std::string, int> text_recognition::get_anno_population(const std::vector<std::pair<std::string, cv::Rect>>& ocr_result)
{
	std::pair<std::string,std::regex> keywords[] = { 
		std::make_pair("Farmers", std::regex(".*Far?mers.*")),
		std::make_pair("Workers", std::regex(".*Workers.*")),
		std::make_pair("Artisans", std::regex(".*Artisans.*")),
		std::make_pair("Engineers", std::regex(".*Engineers.*")),
		std::make_pair("Investors", std::regex(".*Investors.*")),
		std::make_pair("Jornaleros", std::regex(".*aleros.*")),
		std::make_pair("Obreros", std::regex(".*Obreros.*"))};

	std::map<std::string, int> ret;

	for (const auto& kw : keywords) {
		for (const auto& pop_name_word : ocr_result) {
			if (std::regex_match(pop_name_word.first, kw.second)) {
				for (const auto& pop_value_word : ocr_result) {
					if (pop_name_word == pop_value_word)
						continue;
					if (std::abs(pop_value_word.second.tl().y - pop_name_word.second.tl().y) < 2 
						&& pop_value_word.second.tl().x > pop_name_word.second.tl().x) {
						std::string number_string = std::regex_replace( pop_value_word.first, std::regex("\\,|\\."), "");
						int population = std::stoi(number_string);
						if (population > 0) {
							auto insert_result = ret.insert(std::make_pair(kw.first, population));
							std::cout << "new value for " << kw.first << ": " << population << std::endl;
							if (!insert_result.second)
								std::cout << "collision! value already existed: " << insert_result.first->second << std::endl;
						}

					}
				}
			}
		}
	}
	std::cout << "final map: " << std::endl;
	std::for_each(ret.begin(), ret.end(), [](const auto& entry) { std::cout << entry.first << ": " << entry.second << std::endl; });
	return ret;
}

std::shared_ptr<tesseract::TessBaseAPI> text_recognition::ocr()
{
	if (!ocr_) {
		ocr_.reset(new tesseract::TessBaseAPI());
		//auto     numOfConfigs = 1;
		//auto     **configs = new char *[numOfConfigs];
		//configs[0] = (char *) "E:/libs/Tesseract-OCR/tessdata/configs/bazaar";
		if (ocr_->Init(NULL, "eng"));// , configs, numOfConfigs, nullptr, nullptr, false))
		{
			std::cout << "error initialising tesseract" << std::endl;
		}
		ocr_->SetVariable("CONFIGFILE", "bazaar");
	}
	return ocr_;
}
