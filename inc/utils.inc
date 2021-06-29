<?php

/* checks file ownership and updates permissions on specified files, if possible */
function update_file_permissions($file, $permission) {
    /* get the effective uid for error message */
    $e_id = posix_geteuid();
    $o_id = fileowner($file);

    /* if the file uid is the same as effective uid, update file permissions 
       else throw a warning */
    if($e_id == $o_id) {
        chmod($file, $permission);
    }
    else {
        status(STATUS_INFO, "Warning: Unable to update file permissions for $file.");
    }
}

/* performs basic verification on ASN and returns a numeric and string version */
function check_asn_arg($asn, &$asn_number, &$asn_string) {

    if (preg_match("/^AS[0-9]+/i", $asn)) {
        $asn_string = strtoupper($asn);
        $asn_number = substr($asn, 2, strlen($asn));
        return TRUE;
    }
    else {
        if (preg_match("/^[0-9]+/", $asn)) {
            $asn_number = $asn;
            $asn_string = "AS".$asn;
            return TRUE;
        }
    }

    return FALSE;
}

/* checks if the effective user can write to the specified file */
function check_file_perms($file_name) {
    /* get the effective uid for error message */
    $e_id  = posix_geteuid();
    $e_tmp = posix_getpwuid(posix_geteuid());
    $e_user_name = $e_tmp['name'];
    unset($e_temp);

    if(file_exists($file_name) && !is_writeable($file_name)) {
        status(STATUS_ERROR, "Can not write to $file_name, aborting.  Check that file permissions are rw (6) for user $e_user_name.");
        exit(1);
    }

    return(TRUE);
}

/* concatenates srcFile1 and srcFile2 to dstFile */
function concat_files($src_file1, $src_file2, $dst_file) {
    status(STATUS_DEBUG, "Concatinating files $src_file1 + $src_file2 = $dst_file");
    $content_file1 = $content_file2 = null;

    if (!$dst_fp = @fopen("$dst_file", 'w+')) {
        status(STATUS_ERROR, "Error: Unable to open $dst_file for writing, aborting.");
        return(FALSE);
    }

    if (file_exists($src_file1)) {
        $content_file1 = file_get_contents("$src_file1");
    }
    else {
        status(STATUS_INFO, "Warning: Unable to read $src_file1.");
    }

    if (file_exists($src_file2)) {
        $content_file2 = file_get_contents("$src_file2");
    }
    else {
        status(STATUS_INFO, "Warning: Unable to read $src_file1.");
    }

    if (@fwrite($dst_fp, $content_file1) === FALSE) {
        status(STATUS_INFO, "Warning: File content $src_file1 not written to $dst_file.");
    }

    if (@fwrite($dst_fp, $content_file2) === FALSE) {
        status(STATUS_INFO, "Warning: File content $src_file2 not written to $dst_file.");
    }

    fclose($dst_fp);
}


?>
