#!/usr/bin/perl -w 

use Getopt::Long;
use Data::Dumper;
use File::Path;
use Env;

#-----------------------------------------------------------------------------#
## name of subroutine  : by_nat
## description of func : format convert
## example: 
## foreach my $field (sort by_nat @array){...}
##-----------------------------------------------------------------------------#
sub by_nat {
    ##-----------------------------------------------------------------------------#
    my $a1 = $a;
    my $b1 = $b;

    $a1 =~ s/DsAcl(\d)Ingress/DsAclIngress$1/;
    $a1 =~ s/DsAcl(\d)Egress/DsAclEgress$1/;
    $a1 =~ s/DsAcl(\d)HalfIngress/DsAclHalfIngress$1/;
    $a1 =~ s/DsAcl(\d)HalfEgress/DsAclHalfEgress$1/;

    $b1 =~ s/DsAcl(\d)Ingress/DsAclIngress$1/;
    $b1 =~ s/DsAcl(\d)Egress/DsAclEgress$1/;
    $b1 =~ s/DsAcl(\d)HalfIngress/DsAclHalfIngress$1/;
    $b1 =~ s/DsAcl(\d)HalfEgress/DsAclHalfEgress$1/;

    my $aa  = $a1;
    my $bb  = $b1;
    my $aaa = "_".$a1."_";
    my $bbb = "_".$b1."_";

    while($aa =~ /(\d+)/g) {
        my $num =  unpack("B32", pack("N", $1));
        $aaa =~ s/(\D)$1(\D)/$1$num$2/g;
    }
    while($bb =~ /(\d+)/g) {
        my $num =  unpack("B32", pack("N", $1));
        $bbb =~ s/(\D)$1(\D)/$1$num$2/g;
    }

    $aaa cmp $bbb;
}


$curdir = `pwd`;
$runner = `whoami`;
chomp $runner;
chomp $curdir;

# DRV_DEF_D(Aneth, 40, AnethAdvAbility1, SLICE_Default, Op_Default | Op_DisBurst | Op_DisWriteMask, 1, 7, 0x00140b00)
# DRV_DEF_F(Aneth, 40, AnethAdvAbility1, cfgAdvAbility, 48, {1,0,16}, {0,0,32})

$format1 = 'DRV_DEF_D';
$format2 = 'DRV_DEF_F';

$ndb = {};

#push @files,  "ee.h";
#push @files,  "drv_ds_dt2.h";
#$dt2='dt2';
push @files,  "drv_ds_tm.h";
#$tm='tm';

foreach $file (@files) {

#    print "file = $file\n";
#    print "---------------------------------------------\n";
    open (TXT, "$file") || die "unable to open $file\n";

    while(<TXT>) {
        chomp;
        if ($_ =~ /$format2/) 
        {
            @args =  split /,/, $_;
            @str = @args;
            shift @str;
            shift @str;
            shift @str;
            shift @str;

            $r = $args[2];
            $f = $args[3];
            $r =~ s/^\s+|\s+$//;
            $f =~ s/^\s+|\s+$//;
            $tail = join(',', @str);


            if ($_ =~ /DRV_DEF_F._Dynamic/ || $_ =~ /DRV_DEF_F._FlowTcam/)
            {
                $n = $f;

                if($f =~ /^u\d+_g\w+_/ && $f !~ /^u\d+_gTunnel/)
                {
                    $n =~ s/^u\d+_g\w+_//;

                    if(exists($ndb->{$r}{field}{$n})) 
                    {
                        print "$args[0],$args[1],$args[2],$args[3],$args[3],$tail\n";
                    } 
                    else
                    {
                        print "$args[0],$args[1],$args[2], $n,$args[3],$tail\n";
                    }

                    $ndb->{$r}{field}{$n}{value} = $f;
                }
                else
                {
                    $ndb->{$r}{field}{$f}{value} = $f;
                print "$args[0],$args[1],$args[2],$args[3],$args[3],$tail\n";
                }
            }
            else
            {
                $ndb->{$r}{field}{$f}{value} = $f;
                print "$args[0],$args[1],$args[2],$args[3],$args[3],$tail\n";
            }

        } else 
        {
            print "$_\n"
        }
    }
    close TXT
}



