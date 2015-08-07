<?php

function check_file_perms($file_name) {
	/* get the effective uid for error message */
	$e_id = posix_geteuid();
	$e_tmp = posix_getpwuid(posix_geteuid());
	$e_user_name = $e_tmp['name'];
	unset($e_temp);

    if(file_exists($file_name) && !is_writeable($file_name)) {
        status(STATUS_ERROR, "Can not write to $file_name, aborting.  Check that file permissions are rw (6) for user $e_user_name.");
        exit(1);
    }

	return(TRUE);
}

?>
