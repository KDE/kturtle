#!/usr/bin/perl -w

#Copyright (C) 2004 Rafael Beccar <rafael.beccar ! kdemail.net>
#
#This program is free software; you can redistribute it and/or
#modify it under the terms of version 2 of the GNU General
#Public License as published by the Free Software Foundation.
#
#This program is distributed in the hope that it will be useful,
#but WITHOUT ANY WARRANTY; without even the implied warranty of
#MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
#GNU General Public License for more details.
#
#You should have received a copy of the GNU General Public
#License along with this program; if not, write to the
#Free Software Foundation, Inc.,
#59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
#

# 
# Author: Rafael Beccar <rafael.beccar ! kdemail.net>
# 
# Bs.As. November 2004
#
# Automagically generate localized KTurtle logo code.
#
# Usage: ./i18n_examples.pl logokeywords.YOURLANGCODE.xml
# 
# You will need the following under current directory:
# - logokeywords.en_US.xml
# - logokeywords.YOURLANGCODE.xml 
# - The en_US KTurtle examples 
# - This script
#
# e.g.: 
# $./i18n_examples.pl logokeywords.es.xml
#
# arrow.logo -> arrow.es.LOGO
# canvascolors.logo -> canvascolors.es.LOGO
# curly.logo -> curly.es.LOGO
#
# NOTES:
# - Commented lines, variable names, and strings like
# "What's your name?" must be translated by hand.
# - You might want to rename the resulting files to something 
# into your own language e.g. arrow.es.LOGO to flecha.LOGO
# - Please, before executing make sure that you don´t have 
# *.logo files in other language than english_US under current
# directory. If you had any, you should rename them in such a
# way they don't finish in .logo (eg. my.logo to my.LOGO is OK).
#  

use strict;

my @en_commands;
my $langcode;
my @mylang_commands;
my %commands;
my $i;
my @examples= glob "*.logo";
my $filename;
my $line;


#Get the langcode we are translating to
$_ = $ARGV[0];
if (/logokeywords.(\w+).xml/){
	$langcode = $1;
	}

#Get en_US commands
open EN_COMMANDS, "<logokeywords.en_US.xml"
	or die "Cannot open logokeywords.en_US.xml";
$i=0;
while (<EN_COMMANDS>){
	if (/<keyword>(.*)<\/keyword>/){
		$en_commands[$i]=$1;
		$i++;
		}
	}

close EN_COMMANDS;

#Get commands for the given language
$i=0;
while(<>){
	if (/<keyword>(.*)<\/keyword>/){
		$mylang_commands[$i]=$1;
		$commands{$en_commands[$i]}=$mylang_commands[$i];
		$i++;
		}
	}
	
	
#Remove brackets from @en_commands (they only bring troubles) 
#Brackets will be managed in a different way later
shift @en_commands;
shift @en_commands;
	

#Parse *.logo and generate the homologous for the given language
foreach (@examples){
	print "$_ -> "; #The user get some output
	if (/^(\w+).logo/){$filename="$1".".$langcode."."LOGO"};
	print "$filename\n"; #The user get some output
	open EXAMPLE,"<$_"
		or die "Cannot open $_ \n";
	while (<EXAMPLE>){
		#Manage comments (they do not get translated)
		if (/^#(.*)/){
			open TRANSLATION,">>$filename";
			select TRANSLATION;
			print "#$1\n";
			select STDOUT;}
		#Manage tabulated "for .. to .." lines
		elsif (/(^\s+)(\bfor\b)(.*)(\bto\b)/){	
			chomp ($line = "$commands{$2} $3 $commands{$4} $'");
			open TRANSLATION, ">>$filename";
			select TRANSLATION;
			print "$line \n"; 	
			select STDOUT;}
		
		#Manage any other tabulated line
		elsif (/(^\s+)(\w+)/){
			if ($commands{$2}){
				chomp ($line = "$commands{$2} $'");
				}
			elsif (/([=])(\s+)(\w+)/){
				if ($commands{$3}){
					chomp ($line = "$`$1$2$commands{$3}$'");
					}
				}	
			elsif (/([=])(\w+)/){
				if ($commands{$2}){
					chomp ($line = "$` $1 $commands{$2}$'");
					}
				}	
			else
				{
				chomp ($line = "$2 $'");
				}	
			open TRANSLATION,">>$filename";
			select TRANSLATION;
			print "\t$line\n";
			select STDOUT;}
		#Manage "for .. to .." lines
		elsif (/(^\bfor\b)(.*)(\bto\b)/){	
			chomp (my $line = "$commands{$1} $2 $commands{$3} $'");
			open TRANSLATION, ">>$filename";
			select TRANSLATION;
			print "$line \n"; 	
			select STDOUT;}
		#Manage any other line of code
		elsif (/(^\w+)/){
			if ($commands{$&}){
				chomp ($line = "$commands{$&} $'");
				}
			elsif (/([=])(\s+)(\w+)/){
				if ($commands{$3}){
				chomp ($line = "$`$1$2$commands{$3}$'");
					}
				}	
			elsif (/([=])(\w+)/){
				if ($commands{$2}){
					chomp ($line = "$` $1 $commands{$2}$'");
					}
				}	
			else{
				chomp ($line = "$1 $'");
				}	
			open TRANSLATION, ">>$filename";
			select TRANSLATION;
			print "$line \n"; 	
			select STDOUT;
			}
		#Manage brackets
		elsif (/\p{IsPunct}/){
			open TRANSLATION,">>$filename";
			select TRANSLATION;
			print "$&\n";
			select STDOUT;}
		#Manage empty lines
		elsif (/^\s+/){
			open TRANSLATION,">>$filename";
			select TRANSLATION;
			print "\n";
			select STDOUT;}

		}	
	}


