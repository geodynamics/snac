#!/usr/bin/perl

use Data::Dumper;

use lib '..';
use Dictionary;

my $config = Dictionary->new();

print Dumper( $config );

