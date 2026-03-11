# **repman - Report Manager**
**Product of TXStudio**

## usage: `repman <flags> <token>`  
Register token at ~/.local/share/repman/data.json, or use `repman repman`.  
Use token to open the according file.  

## Flags:
| | |
|-|-|
`-h` / --help`			| show the help message
`-l` / `--list`			| show the registered token and their according file path
`--list-cmd`			| show the registered command of launching
`-x` / `--command`		| assign specific editor. edit options in ~/.config/repman/config.json or use `repman config` -> "Editors"

## Config File:
### commands
The commands declared here are the commands tha would be ran when specified a command by `-c` or `--command`.  
In each command, **there have to be a `$FILE` keyword to get the file path of the requesting file**.