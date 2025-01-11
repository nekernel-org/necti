# !/bin/zsh

mkdir -p html

XSDocgen --source ./dev/ --output ./html --undocumented --project-name "ZKA CoreFoundation" --enable-c++ --project-copyright "t& Corporation. &copy; %Y - All rights Reserved" --project-version "930.2024.1" --company-name "t& Corporation."
