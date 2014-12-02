#!/usr/bin/perl
use Encode;
use LWP;
use LWP::UserAgent;
use LWP::Simple;
use HTTP::Cookies;
##
# Perl script to parse user text data
# and give some statistic report
##

############################Constants############################
$LEADING_URL = "http://www.ip138.com:8080/search.asp?";

# .csv stored the existed num_list, whose format is like:
#
# 13022345678,Jiangsu
# 18900002345,Beijing
#
$PRELOAD = "preload.csv";

$FULLUSER = "fulluser.txt";
$ACTIVEUSER = "activeuser.txt";
$INACTIVEUSER = "inactiveuser.txt";
$TEXRESULT = "report.tex";

##
# num_list :
#
# {13x xxxx xxxx} => "Province1"
# {13x xxxx xxxx} => "Province2"
# {13x xxxx xxxx} => "Province3"
# ....        ....       ....
our %num_list;

##
# prov_list - account how many provinces
# {Province1} => 30
# {Province2} => 98
# {Province3} => 3
#  ...          ...
our %prov_list;

##
# Active user's province list
#
our %active_prov_list;
our $active_len;
our %inactive_prov_list;
our $inactive_len;

################################################################

# stored the latest mobile=>Province into .csv as preloaded for
# next time's parsing.
sub update_csvdata() {
    my ($myl) = (@_);
    # remmember to encode it as UTF-8
    open(CSV, ">$PRELOAD") or die "failed try writing $PRELOAD\n";
    foreach $k (keys $myl){
        if($k =~ /1\d{8}/) {
            $v = ${$myl}{$k};
#printf("[DEBUG] the key:$k, the val:$v\n");
            print CSV "$k,$v\n";
      } else {
          printf("ATTENTION, found an invalid key, ignore update it to CSV...\n");
      }
    }
    close(CSV);
}

# try fill up the num_list, and store it into a hash
sub get_num_list() {
    my ($myh) = (@_); # input param is actually a reference of Hash!

    my $UA = LWP::UserAgent->new(keep_alive=>1);
    $UA->agent("Mozilla/5.0 (Windows NT 6.1; WOW64) AppleWebKit/537.36 (KHTML, like Gecko) Chrome/37.0.2062.124 Safari/537.36");

    #First try get all existed records from a preload csv file.
    open(CSV, "<:encoding(utf-8)", "$PRELOAD") || die "failed open preload csv";
    while(<CSV>){
        @lines = split(",");
        chomp($lines[1]);
        ${$myh}{$lines[0]} = $lines[1];
    }
    close(CSV);

    printf("Finished the preload CSV reading...\n");

    #next, read fulluser data file...
    open(FULL, $FULLUSER) || die "failed open full user data";
    while(<FULL>){
        $line = $_;
        chomp($line); # here store one mobile number, like 13022592323

        unless (exists ${$myh}{$line}) {
            printf("$line didn't existed in the preload.csv, will get it from internet...\n");
            my $url = $LEADING_URL . "mobile=" . $line . "&action=mobile";

            my $res = $UA->get($url);
            if($res->is_success) {
                my $html = $res->content;
                open(TMPFILE, ">", "get.html") or die "failed write the html";
                print TMPFILE $html;
                close(TMPFILE);

                open(TMPFILE, "get.html") || die "failed open the temp html";
                $TXT_ANCHOR = "ø®∫≈πÈ Ùµÿ";
                while(<TMPFILE>) {
                    if($_ =~ /\Q$TXT_ANCHOR\E.*class=tdc2\>\<.*--\>(.*)\<\/TD\>/) {
                        $h = $1;
                        if($h =~ /(.*)\&nbsp;(.*)/) {
                            $o = $1;
                            printf("[DEBUG] the raw html data:$o\n");
                            $out = decode("gbk", $o);
                            printf("[DEBUG] after gdk html data:$out\n");
                            $o = encode("utf-8", $out);

                            ${$myh}{$line} = $o; # we store as UTF-8

                            printf("\t$line ==> $o\n");
                            last; #remember , last is C's break;
                        }
                    } elsif($_ =~ /noswap\>\Q$TXT_ANCHOR\E\<\/TD\>/ || $_ =~ /\Q$TXT_ANCHOR\E/) {
                        $newline = 1;#result stored at next new line
                    } elsif($newline == 1) {
                        $newline = 0;
                        if($_ =~ /\>(.*)\&nbsp;(.*)\<\/TD\>/){
                            $o = $1;
                            printf("[DEBUG2] the raw html data:$o\n");
                            $out = decode("gbk", $o);
                            printf("[DEBUG2] after gdk html data:$out\n");
                            $o = encode("utf-8", $out);

                            ${$myh}{$line} = $o;

                            printf("\t$line ==> $o\n");
                            last;
                        }
                    }
                }
                close(TMPFILE);
            }
        }
    }
    close(FULL);
}

# process the num_list, and give out a new list which contained
# province info, stored in global %prov_list
sub get_prov_list(){
    my($myh) = (@_);

    # full province list
    foreach $k (keys $myh) {
        unless (exists ${$myh}{$k}) {
            $prov_list{${$myh}{$k}} = 1;#first new province
        } else {
            $prov_list{${$myh}{$k}} ++;
        }
    }


    my %tmpactive;
    open (ACTIVE, "$ACTIVEUSER") || die "failed open active user data";
    while(<ACTIVE>) {
        $line = $_;
        chomp($line);

        if(exists ${$myh}{$line}) {
            $tmpactive{$line} = ${$myh}{$line};
        }
    }
    close(ACTIVE);

    $active_len = keys %tmpactive;

    foreach $ky (keys %tmpactive) {
        unless (exists $active_prov_list{$tmpactive{$ky}}) {
            $active_prov_list{$tmpactive{$ky}} = 1;
        } else {
            $active_prov_list{$tmpactive{$ky}} ++;
        }
    }

    my %tmpinactive;
    open(INACTIVE, "$INACTIVEUSER") || die "failed open inactive user data";
    while(<INACTIVE>) {
        $line = $_;
        chomp($line);

        if(exists ${$myh}{$line}) {
            $tmpinactive{$line} = ${$myh}{$line};
        }
    }
    close(INACTIVE);

    $inactive_len = keys %tmpinactive;

    foreach $ky (keys %tmpinactive) {
        unless (exists $inactive_prov_list{$tmpinactive{$ky}}) {
            $inactive_prov_list{$tmpinactive{$ky}} = 1;
        } else {
            $inactive_prov_list{$tmpinactive{$ky}} ++;
        }
    }
}


# export TeX section for plot the analysis result
sub export_tex_section() {
    my @sortedlist;
    my $count=0;

    open(TEX, ">:encoding(utf-8)", "$TEXRESULT") || die "failed create the \'$TEXRESULT\' file\n";

    print TEX "\\begin{bchart}\n";
    foreach $k (sort {$prov_list{$b} <=> $prov_list{$a}} keys %prov_list){
        $count++;
        if($count == 1) {
            print TEX "\\bcbar[text=$k,color=red]{$prov_list{$k}}\n";
        } elsif($count == 2) {
            print TEX "\\bcbar[text=$k,color=orange]{$prov_list{$k}}\n";
        } elsif($count == 3) {
            print TEX "\\bcbar[text=$k,color=green!60!blue]{$prov_list{$k}}\n";
        }else {
            print TEX "\\bcbar[label=$k]{$prov_list{$k}}\n";
        }
    }
    print TEX "\\end{bchart}\n";

    print TEX "\n\nBelow are active pie and sorted list...\n\n";

    $fulllen = keys %num_list;
    $medialen = ($fulllen - $active_len - $inactive_len);

    $ratio_active = $active_len / $fulllen;
    printf("$active_len / $fulllen , ratio action: $ratio_active\n");

    $count = 0;
    print TEX "\\begin{bchart}\n";
    foreach $ky (sort {$active_prov_list{$b} <=> $active_prov_list{$a}} keys %active_prov_list) {
        $count++;
        if($count <= 3) {
            print TEX "\\bcbar[text=$ky]{$active_prov_list{$ky}}\n";
        } else {
            print TEX "\\bcbar[label=$ky]{$active_prov_list{$ky}}\n";
        }
    }
    print TEX "\\end{bchart}\n";

    print TEX "\n\nBelow are inactive pie and sorted list...\n\n";
    $count = 0;
    print TEX "\\begin{bchart}\n";
    foreach $ky (sort {$inactive_prov_list{$b} <=> $inactive_prov_list{$a}} keys %inactive_prov_list) {
        $count++;
        if($count <= 3) {
            print TEX "\\bcbar[text==$ky]{$inactive_prov_list{$ky}}\n";
        } else {
            print TEX "\\bcbar[label=$ky]{$inactive_prov_list{$ky}}\n";
        }
    }
    print TEX "\\end{bchart}\n";

    $full = keys %num_list;
    printf ("full users:$full\n");

    close(TEX);
}



##
# Main Entry Point Start


#binmode(STDOUT, ':encoding(utf8)');

&get_num_list(\%num_list);
$len = keys %num_list;
printf("After call get_num_list, the whole users info account = $len\n");

&update_csvdata(\%num_list);
printf("All user data stored in $PRELOAD\n");

&get_prov_list(\%num_list);
#after above function call, %prov_list stored the
#calculated province list data.

&export_tex_section();

