#include "Project.hpp"


// template <class T>
// class MutableArrView {
// public:
// 	using It_t = std::vector<T>::iterator;

// 	MutableArrView(It_t begin, It_t end) : m_begin(begin), m_end(end) {}
// 	MutableArrView(std::vector<T>& vec, size_t begin, size_t end) : m_begin(vec.begin() + begin), m_end(vec.begin() + end) {}

// 	T& operator[](size_t index) { return *(m_begin + index); }
// 	size_t size() const { return static_cast<size_t>(m_end - m_begin); }
// 	It_t begin() const { return m_begin; }
// 	It_t end() const { return m_end; }

// private:
// 	It_t m_begin, m_end;
// };
// template <class T>
// class ConstArrView {
// public:
// 	using ConstIt_t = std::vector<T>::const_iterator;

// 	ConstArrView(ConstIt_t begin, ConstIt_t end) : m_begin(begin), m_end(end) {}
// 	ConstArrView(const std::vector<T>& vec, size_t begin, size_t end) : m_begin(vec.begin() + begin), m_end(vec.begin() + end) {}

// 	const T& operator[](size_t index) const { return *(m_begin + index); }
// 	size_t size() const { return static_cast<size_t>(m_end - m_begin); }
// 	ConstIt_t begin() const { return m_begin; }
// 	ConstIt_t end() const { return m_end; }

// private:
// 	ConstIt_t m_begin, m_end;
// };


class Parameter {
public:
	Parameter(const std::string& in_flag, std::span<std::string> in_arg)
	    : flag(in_flag), arg(std::move(in_arg)) {}
	const std::string& flag;
	std::span<std::string> arg;
};
class ParameterList {
public:
	ParameterList(std::vector<std::string>&& in_data) : data(std::move(in_data)), data_span(data) { init(); }
	ParameterList(const std::vector<std::string>& in_data) : data(in_data), data_span(data) { init(); }

	void init() {
		for (size_t i = 0; i < data.size(); ++i) {
			if (isFlag(data[i])) {
				flagPoints.push_back(i);
			}
		}
	}
	size_t size() const { return flagPoints.size(); }
	Parameter operator[](size_t index) const {
		return Parameter{
			data[flagPoints[index]],
			index + 1 >= flagPoints.size() ?
			    data_span.subspan(flagPoints[index] + 1) :
			    data_span.subspan(flagPoints[index] + 1, flagPoints[index + 1] - flagPoints[index])
		};
	}

	bool isFlag(const std::string& str) {
		if (str.size() > 1) {
			return str[0] == '-';
		}
		return 0;
	};

	std::vector<std::string> data;
	std::vector<size_t> flagPoints;
	std::span<std::string> data_span;
};

class Repman {
public:
	Repman() {
		std::fs::path dataPath(std::getenv("HOME"));
		std::fs::path configPath = dataPath;
		dataPath = dataPath / ".local/share/repman/data.json";
		if (!std::fs::exists(dataPath)) {
			std::fs::create_directories(dataPath.parent_path());
			std::ofstream ofs(dataPath, std::ios::binary);
			ofs.write(DefaultContent_Data, strlen(DefaultContent_Data));
			ofs.close();
		}
		data = tx::parseJson(tx::readWholeFileText(dataPath));

		configPath = configPath / ".config/repman/config.json";
		if (!std::fs::exists(configPath)) {
			std::fs::create_directories(configPath.parent_path());
			std::ofstream ofs(configPath, std::ios::binary);
			ofs.write(DefaultContent_Config, strlen(DefaultContent_Config));
			ofs.close();
		}
		config = tx::parseJson(tx::readWholeFileText(configPath));

		command = config["defaultCommand"].get<std::string>();
	}

	// --help
	void help() {
		cout << HelpMessage;
	}
	void run(const ParameterList& paramls) {
		// apply params
		bool terminate = 0;
		size_t index = 0;
		for (int i = 0; i < paramls.size(); ++i) {
			if (!FlagMap.exist(paramls[i].flag)) {
				terminate = 1;
				cout << "Unknow flag: \"" << paramls[i].flag << "\"\nterminating...\n";
				break;
			}
			terminate = executeFlag_impl(FlagMap.at(paramls[i].flag), paramls[i].arg, index);
			if (terminate) break;
		}

		if (terminate) return;
		for (int i = index; i < paramls.data.size(); ++i) {
			std::string filePath;
			if (getFile(paramls.data[i], filePath)) continue;
			run_impl(filePath);
		}
		if (index == paramls.data.size()) {
			cout << "No token was provided. No file will be open.\n";
		}
	}

private:
	enum class Flag {
		SetCommand, // --command
		Help,
		List,
		ListCmd
	};

private:
	tx::JsonObject data;
	tx::JsonObject config;

	// runtime variables
	std::string command;

	void run_impl(std::string filePath) {
		std::string cmd = command;
		size_t index = cmd.find("$FILE");
		if (index == std::string::npos) {
			cout << "Invalid command: \"" << cmd << "\": Command didn't include `$FILE`.";
			return;
		}
		cmd.replace(index, 5, filePath);
		system(cmd.c_str());
	}

	bool executeFlag_impl(Flag flag, std::span<std::string> args, size_t& index) {
		switch (flag) {
		case Flag::Help:
			help();
			return 1;
		case Flag::List:
			list();
			return 1;
		case Flag::ListCmd:
			listCmd();
			return 1;
		case Flag::SetCommand:
			if (args.empty()) {
				cout << "--command requires a value\n";
				return 1;
			}
			const string& name = args[0];
			const tx::JsonObject editors = config["commands"].get<tx::JsonObject>();
			if (!editors.exist(name)) {
				cout << "Unknown command: \"" << name << "\"\nterminating...\n";
				return 1;
			}
			command = editors[name].get<std::string>();
			index += 2;
		}
		return 0;
	}
	bool getFile(const std::string& token, std::string& filePath) {
		if (!data.exist(token)) {
			cout << "Unknown token: \"" << token << "\". this file will not be opened.";
			return 1;
		}
		filePath = data[token].get<std::string>();
		return 0;
	}

	// functionalities

	// list all token entries
	void list() {
		for (int i = 0; i < data.size(); i++) {
			cout << '"' << data.atIndex(i).k() << '"' << ": \"" << data.atIndex(i).v().get<string>() << "\"\n";
		}
	}
	// list all commands
	void listCmd() {
		tx::JsonObject commands = config["commands"].get<tx::JsonObject>();
		for (int i = 0; i < commands.size(); i++) {
			cout << '"' << commands.atIndex(i).k() << '"' << ": \"" << commands.atIndex(i).v().get<string>() << "\"\n";
		}
	}

private: // hard coded data
	inline static tx::KVMap<std::string, Flag> FlagMap = {
		{ std::string("--command"), Flag::SetCommand },
		{ std::string("-c"), Flag::SetCommand },
		{ std::string("--help"), Flag::Help },
		{ std::string("-h"), Flag::Help },
		{ std::string("--list"), Flag::List },
		{ std::string("-l"), Flag::List },
		{ std::string("--list-cmd"), Flag::ListCmd }
	};

	inline static constexpr const char* DefaultContent_Data = R"({
	"repman": "/home/TX_Jerry/.local/share/repman/data.json",
	"data": "/home/TX_Jerry/.local/share/repman/data.json"
})";
	inline static constexpr const char* DefaultContent_Config = R"({
	"defaultCommand": "code 2>/dev/null \"$FILE\"",
	"commands": {
		"Code": "code 2>/dev/null \"$FILE\"",
		"code": "code 2>/dev/null \"$FILE\"",
		"VSCode": "code 2>/dev/null \"$FILE\"",
		"vscode": "code 2>/dev/null \"$FILE\"",
		"nano": "nano \"$FILE\""
	}
})";
	inline static constexpr const char* HelpMessage = R"(repman - Report Manager
Product of TXStudio

usage: `repman <flags> <token>`
Register token at ~/.local/share/repman/data.json, or use `repman repman`.
Use token to open the according file.

Flags:
-h / --help			show the help message
-l / --list			show the registered token and their according file path
     --list-cmd		show the registered command of launching
-e / --editor		assign specific editor. edit options in ~/.config/repman/config.json or use `repman config` -> "Editors"
)";
};



int main(int argc, char* argv[]) {

	Repman app;
	if (argc <= 1) { // no arg
		app.help();
	} else {
		std::vector<std::string> args(argv + 1, argv + argc);
		app.run(ParameterList(args));
	}
	return 0;
}

// things to add:
// 1. txjson: add append
// 2. add append functionality
// 3. txjson: add iterator