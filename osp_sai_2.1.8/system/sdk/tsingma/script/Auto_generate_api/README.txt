This text show how to use scrpt in this folder

#1. auto_generate_api.sh can generate api for all centec chips
    if you want to only generate one chip's api, you can use the
    command "sh auto_generate_api.sh chipname", for example:
    "sh auto_generate_api.sh humber"

    if you want to generate a mixed api, you can use the command
    "sh auto_generate_api.sh chiname1 chiname2 ...", for example
    "sh auto_generate_api.sh humber greatbelt"

#2. generate_api_txt.tcl and generate_api.tcl are called by
    auto_generate_api.sh
