<?
 ;

include_once("config.inc.php");

function replace_vars($input, $vars, $pref, $ovrdext="") {
    foreach ($vars as $pos => $var) {
	if ($ovrdext) {
	    $var = substr($var,0,-strlen($ovrdext)).$ovrdext;
	}
	// $input = str_replace($var,$pref.$var,$input);
	$input = ereg_replace("([^a-zA-Z0-9_-])$var([^a-zA-Z0-9_-])","\\1$pref$var\\2",$input);
    }
    return $input;
}

function filetrim($file) {
    $lines = file($file);
    foreach ($lines as $pos => $line)
	$lines[$pos] = trim($line);
    return $lines;
}

function is_header($file) {
    global $headers;
    return (is_int(array_search($file, $headers)));
}

function is_source($file) {
    global $source;
    return (is_int(array_search($file, $source)));
}



$files = filetrim("files.csv");
$headers = filetrim("header.csv");
$source = filetrim("source.csv");
$symbols = filetrim("symbols.csv");


foreach($files as $pos => $file) {
    $lines = file("$INDIR/".$file);
    $output = "";

    if (is_source($file) || is_header($file)) {
	foreach ($lines as $pos => $line) {
	    $line = replace_vars($line, $headers, $PREFIX);
	    $line = replace_vars($line, $symbols, $PREFIX);
	    $output .= $line;
	}
	$filename = "$OUTDIR/".$PREFIX.$file;
    }
    else {
	// files not being source are concidered only to contain references
	// to header source or objectfiles (such as Makefiles)
	foreach ($lines as $pos => $line) {
	    $line = replace_vars($line, $headers, $PREFIX);
	    $line = replace_vars($line, $source, $PREFIX);
	    $line = replace_vars($line, $source, $PREFIX, ".o");
	    // $line = str_replace($DEVNAME.".a",$PREFIX.$DEVNAME.".a",$line);
	    $line = replace_vars($line, array($DEVNAME), $PREFIX);
	    $output .= $line;
	}
	$filename = "$OUTDIR/".$file;
    }
    
    $of = fopen($filename, "w");
    fwrite ($of, $output);
    fclose($of);
}

?>
