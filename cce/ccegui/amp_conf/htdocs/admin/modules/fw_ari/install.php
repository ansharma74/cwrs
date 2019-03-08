<?php
if (!defined('FREEPBX_IS_AUTH')) { die('No direct script access allowed'); }
// This file is part of FreePBX.
//
//    FreePBX is free software: you can redistribute it and/or modify
//    it under the terms of the GNU General Public License as published by
//    the Free Software Foundation, either version 2 of the License, or
//    (at your option) any later version.
//
//    FreePBX is distributed in the hope that it will be useful,
//    but WITHOUT ANY WARRANTY; without even the implied warranty of
//    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//    GNU General Public License for more details.
//
//    You should have received a copy of the GNU General Public License
//    along with FreePBX.  If not, see <http://www.gnu.org/licenses/>.
//
//    Copyright 2006 FreePBX
//
// HELPER FUNCTIONS:

function fw_ari_print_errors($src, $dst, $errors) {
	out("error copying files:");
	out(sprintf(_("'cp -rf' from src: '%s' to dst: '%s'...details follow"), $src, $dst));
	freepbx_log(FPBX_LOG_ERROR, sprintf(_("fw_ari couldn't copy file to %s"),$dst));
	foreach ($errors as $error) {
		out("$error");
		freepbx_log(FPBX_LOG_ERROR, _("cp error output: $error"));
	}
}

global $amp_conf;
global $asterisk_conf;

$debug = false;
$dryrun = false;

/** verison_compare that works with freePBX version numbers
 *  included here because there are some older versions of functions.inc.php that do not have
 *  it included as it was added during 2.3.0beta1
 */
if (!function_exists('version_compare_freepbx')) {
	function version_compare_freepbx($version1, $version2, $op = null) {
		$version1 = str_replace("rc","RC", strtolower($version1));
		$version2 = str_replace("rc","RC", strtolower($version2));
		if (!is_null($op)) {
			return version_compare($version1, $version2, $op);
		} else {
			return version_compare($version1, $version2);
		}
	}
}

/*
 * fw_ari install script
 */
	$htdocs_ari_source = dirname(__FILE__)."/htdocs_ari/*";
	$htdocs_ari_dest = $amp_conf['AMPWEBROOT']."/recordings";

	if (!file_exists(dirname($htdocs_ari_source))) {
    out(sprintf(_("No directory %s, install script not needed"),dirname($htdocs_ari_source)));
    return true;
  }

	$msg = _("installing files to %s..");

	// TODO: for some reason the .htaccess is not being copied with the rest????
	$src_file[] = $htdocs_ari_source;
	$src_file[] = dirname($htdocs_ari_source) . "/.htaccess";
	foreach ($src_file as $src) {
		outn(sprintf($msg, $htdocs_ari_dest));
		$out = array();
		exec("cp -rf $src $htdocs_ari_dest 2>&1",$out,$ret);
		if ($ret != 0) {
			fw_ari_print_errors($src, $htdocs_ari_dest, $out);
			out(_("done, see errors below"));
		} else {
			out(_("done"));
		}
	}
	// Make sure that libfreepbx.javascripts.js is available to ARI
	$libfreepbx = $amp_conf['AMPWEBROOT'].'/admin/common/libfreepbx.javascripts.js';
	$dest_libfreepbx = $htdocs_ari_dest.'/theme/js/libfreepbx.javascripts.js'; 
		if (file_exists($libfreepbx) && !file_exists($dest_libfreepbx)) {
		outn(_("linking libfreepbx.javascripts.js to theme/js.."));
		if (link($libfreepbx, $dest_libfreepbx)) {
			out(_("ok"));
		} else {
			out(_("possible error - check warning message"));
		}
	}

	// We now delete the files, this makes sure that if someone had an unprotected system where they have not enabled
	// the .htaccess files or otherwise allowed direct access, that these files are not around to possibly cause problems
	//
	out(_("fw_ari file install done, removing packages from module"));
	unset($out);
	exec("rm -rf $htdocs_ari_source 2>&1",$out,$ret);
	if ($ret != 0) {
		out(_("an error occured removing the packaged files"));
	} else {
		out(_("files removed successfully"));
	}
?>
