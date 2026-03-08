function(tx_cp_dir_to_bin in_target in_dirPath)
	add_custom_command(TARGET ${in_target} POST_BUILD COMMAND ${CMAKE_COMMAND} -E copy_directory ${in_dirPath} $<TARGET_FILE_DIR:${in_target}>)
endfunction()

function(tx_cp_dir_to_bin in_target in_dirPath)
    get_filename_component(folderName "${in_dirPath}" NAME)
    set(dst_dir "$<TARGET_FILE_DIR:${in_target}>/${folderName}")

    add_custom_command(
        TARGET ${in_target}
        POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E make_directory "${dst_dir}"
        COMMAND ${CMAKE_COMMAND} -E copy_directory "${in_dirPath}" "${dst_dir}"
    )
endfunction()