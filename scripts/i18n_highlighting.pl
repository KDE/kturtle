#!/usr/bin/perl -w

# Copyright (C) 2004 Rafael Beccar <rafael.beccar ! kdemail.net>
#
# This program is free software; you can redistribute it and/or
# modify it under the terms of version 2 of the GNU General
# Public License as published by the Free Software Foundation.
#
# This program is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU General Public License for more details.
#
# You should have received a copy of the GNU General Public
# License along with this program; if not, write to the
# Free Software Foundation, Inc.,
# 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.

 
# Author: Rafael Beccar <rafael.beccar ! kdemail.net>
# 
# Bs.As. November 2004
#
# Automagically gen. localized highlighting for KTurtle commands
#
# Usage:
# ./i18n_highlighting.pl logokeywords.YOURLANG.xml 
#
# E.g.
# ./i18n_highlighting.pl logokeywords.es.xml 
#
# NOTES:
# - You'll need logokeywords.YOURLANGCODE.xml under kturtle/scripts
# 
# - The script search for logokeywords.en_US.xml under ../data/
# and writes the resulting files under current directory
#


use strict;

my $langcode;
my @en_commands;
my @mylang_commands;
my %commands;
my $i;
my $line;

#Give message if no parameter passed
if (! $ARGV[0]){
        die "Usage:\n ./i18n_highlighting.pl logokeywords.YOURLANG.xml";}

#Get the langcode we are translating to
$_ = $ARGV[0];
if (/logokeywords.(\w+).xml/){
        $langcode = $1;
                }

#Get en_US commands
open EN_COMMANDS, "<../data/logokeywords.en_US.xml"
	or die "Cannot open ../data/logokeywords.en_US.xml";
$i=0;
while (<EN_COMMANDS>){
	if (/<keyword>(.*)<\/keyword>/){
		$en_commands[$i]=$1;
		$i++;
		}
	elsif (/<alias>(.*)<\/alias>/){
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
	elsif (/<alias>(.*)<\/alias>/){
		$mylang_commands[$i]=$1;
		$commands{$en_commands[$i]}=$mylang_commands[$i];
		$i++;
		}
	}
	
#Remove brackets from @en_commands (they are not needed here) 
#Brackets will be managed in a different way later
shift @en_commands;
shift @en_commands;

#Set destination
my $filename = "logohighlightstyle.${langcode}.xml";


#Parse logohighlightstyle.en_US.xml and generate the homologous for the given language
open HIGHLIGHT,"<../data/logohighlightstyle.en_US.xml"
	or die "Cannot open $_ \n";
while (<HIGHLIGHT>){
	#Manage language tag	
	if (m%(en_US)%){
		open TRANSLATION,">>$filename";
		select TRANSLATION;
		print "$`${langcode}$'";
		select STDOUT;}
	#Manage translatable lines
	elsif (m%<item> (\w+) </item>%){
		if ($commands{$1}){
			$line = "\t<item> $commands{$1} </item>";}
		open TRANSLATION,">>$filename";
		select TRANSLATION;
		print "$line \n";
		select STDOUT;
		}
	#Manage not translatable lines	
	else{
		open TRANSLATION,">>$filename";
		select TRANSLATION;
		print "$_";}
		select STDOUT;
	}
close TRANSLATION;
print "DONE... The translated file is:\n $filename \n";


