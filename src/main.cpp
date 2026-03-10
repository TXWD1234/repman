#include "Project.hpp"



using ParameterList = std::pair<std::vector<std::string>, std::vector<std::vector<std::string>>>;

ParameterList parseParameter(const std::vector<std::string>& args){
	auto isFlag = [](const std::string& str){
		if(str.size() > 1){
			return str[0] == '-';
		} return 0;
	}

	ParameterList params;

	for(int i = 0; i < args.size(); ++i){
		if(isFlag(args[i])){
			params.first.push_back(args[i]);
			params.second.push_back({});
			continue;
		}
		if(!params.first.size()) continue;
		params.second.back().push_back(args[i]);
	}

	return params;
}

class Repman {
public:
	enum class Flag {
		SetFile, // -o
		SetTextEditor // --editor
	};

	void executeFlag(Flag flag, std::string str){
		
	}
private:
	inline static tx::KVMap<std::string, Flag> flagMap = {
		{std::string("-o"), Flag::SetFile},
		{std::string("--editor"), Flag::SetTextEditor}
	};

};




int main(int argc, char* argv[]) {
	std::fs::path dataPath(std::getenv("HOME"));
	dataPath = dataPath / ".local/share/repman/data.json";
	if (!std::fs::exists(dataPath)) {
		std::fs::create_directories(dataPath.parent_path());
		std::ofstream ofs(dataPath, std::ios::binary);
		ofs.write("{\n}", 3);
		ofs.close();
	}
	tx::JsonObject root = tx::parseJson(tx::readWholeFileText(dataPath));
	if (argc <= 1) { // no arg
		// list all entries
		for (int i = 0; i < root.size(); i++) {
			cout << '"' << root.atIndex(i).k() << '"' << ": \"" << root.atIndex(i).v().get<string>() << "\"\n";
		}
	} else {

		std::vector<std::string> args(argv + 1, argv + argc);

		if (root.exist(argv[1])) {
			string cmd = "code \"";
			cmd.append(root[argv[1]].get<string>());
			cmd.push_back('"');
			cout << cmd << endl;
			system(cmd.c_str());
		} else {
			cout << "entry not found.\n";
		}
	}
	return 0;
}

// things to add:
// 1. txjson: add append
// 2. add append functionality
// 3. txjson: add iterator