#!/usr/bin/perl -w
#This program takes a string like: caput+sicut: and converts it into a following sgrep query expression, depending on the macro name provided in the arguments: eg: TITLE_MATCH1("caput") equal TITLE_MATCH1("sicut")
#It is invoked by the C program while constructing the sgrep query (at the last stage)
#The command executed for running this perl script is like this:
# echo 'caput+sicut' | ./stringSub.plx TITLE_MATCH1 1

$macro = $ARGV[0];
$eachTerm = $ARGV[1];  # use macro for every term, or just once for whole

$match = "";
$delim = "";
$rest = "";

if ($eachTerm==1) {$toReturn = "";}
else {$toReturn = "$macro(";}

$_ = <STDIN>;
$_ .= "\n"; # make sure there's a terminator

#input already normalized, so no extra spaces,only properly nested brackets

while( /^([^\)\(\n\+\-\|\"\=]*)([\)\(\n\+\-\|\"\=])/ ){
	
	$match = $1;
	$delim = $2;
	$rest = $';
	
	if($match eq "") {
		if ($delim eq "\"" and $rest =~ /"/) {
			$match = "\"$`\""; # quoted string: allow any contents
			$rest = $';
			$match =~ s/^"\**/"/; # no leading asterisks
			$match =~ s/\**"$/"/; # no trailing asterisks
			if ($match =~ /\*/) { # acts as a wild card
				$match = "INNER($`\"..\"$')";
			}
			while( $match =~ /\*/ ){
				$match = "SPAN($`\"),\"$'"; 
			}
			if($match ne "\"\""){
				if($eachTerm == 1){
					$toReturn .= "$macro($match)";
				}else{
					$toReturn .= "($match)";
				}
			}
		}
		elsif ($delim eq "=" and $rest =~ /^\w+/) {
			$match = "WORD(\"$&\")"; # grab the word to be matched
			$rest = $';
			if($eachTerm == 1){
				$toReturn .= "$macro($match)";
			}else{
				$toReturn .= "($match)";
			}
		}
	}else{
		if($match ne ""){
			if($eachTerm == 1){
				$toReturn .= "$macro(\"$match\")";
			}else{
				$toReturn .= "(\"$match\")";
			}
		}
	}
	if($eachTerm==1){ # to allow boolean searches
		if ($delim eq "+") {$toReturn .= " equal ";}
		elsif ($delim eq "-") {$toReturn .= " not equal ";}
		elsif ($delim eq "|") {$toReturn .= " or ";}
	}else{ #for the sgrep query to color the actual query words
		if ($delim =~ /[\+\-\|]/) {$toReturn .= " or ";}
	}
	if ($delim =~ /[\)\(]/) {$toReturn .= $delim;}
	
	$_ = $rest;
	
}

print $toReturn;
if ($eachTerm==0) {print ")";}
