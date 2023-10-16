set (version_file "${CMAKE_BINARY_DIR}/version_string.tmp")

function(_git_branch branch)
  if(EXISTS "${CMAKE_SOURCE_DIR}/.git/HEAD")
    file(STRINGS "${CMAKE_SOURCE_DIR}/.git/HEAD" git_lines)
    get_filename_component(branch ${git_lines} NAME)
    set(branch ${branch} PARENT_SCOPE)
  endif()
endfunction(_git_branch)

# 
#  _begin_status
#    write temp file for messages
#
function(_begin_status)
  file(WRITE "${version_file}" "")
endfunction(_begin_status)

# 
#  _write_status
#    write message to file and stdout
#
function(_write_status text)
  string(FIND ${text} ": " colon_pos)
  if(NOT colon_pos EQUAL -1)
    math(EXPR nspaces "22 - ${colon_pos}") 
    foreach(sp RANGE ${nspaces})
      string(CONCAT space_splitter ${space_splitter} " ")
    endforeach(sp)
    string(REPLACE ": " ":${space_splitter}" text ${text})
  endif()
#  message(STATUS "${text}")
  file(APPEND "${version_file}" "\n${text}")
endfunction(_write_status)

# 
#  _end_status
#    output temp file with messages to stdout
#
function(_end_status)
  message(STATUS "--------------------------------------")
  file(STRINGS "${version_file}" version_lines)
  foreach(version_line ${version_lines})
    message(STATUS "${version_line}")
  endforeach(version_line)
  message(STATUS "--------------------------------------")
endfunction(_end_status)