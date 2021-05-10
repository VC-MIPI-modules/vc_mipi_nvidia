#!/usr/bin/env  python
# vim: tabstop=4 expandtab shiftwidth=4 softtabstop=4
# Made by Vision Components 2015-2018 (MBE)

import  pygtk
pygtk.require('2.0')
import  gtk

import  socket
import  sys
import  os
import  binascii
import  struct

import  logging
import  tempfile

import datetime
import gobject
import time

from itertools import chain, izip
################################################################################
# ##############################################################################
# #
# #   Scroll down ....

app_icon = [
"32 32 174 2", "  	c None", ". 	c #00427A", "+ 	c #256397", "@ 	c #165892",
"# 	c #134C85", "$ 	c #114B84", "% 	c #074780", "& 	c #04467F", "* 	c #29669A",
"= 	c #094881", "- 	c #00437B", "; 	c #004179", "> 	c #29649E", ", 	c #03528A",
"' 	c #195184", ") 	c #00447C", "! 	c #1F5488", "~ 	c #2A679B", "{ 	c #266498",
"] 	c #164E87", "^ 	c #004F87", "/ 	c #0B4A7D", "( 	c #35649A", "_ 	c #154D86",
": 	c #0E4A83", "< 	c #226195", "[ 	c #346399", "} 	c #11568F", "| 	c #28639D",
"1 	c #DFE7F0", "2 	c #F9F6FB", "3 	c #EDF2F5", "4 	c #EEF3F6", "5 	c #FAFCF9",
"6 	c #C1CDDB", "7 	c #487AA4", "8 	c #F4F9FC", "9 	c #4171A1", "0 	c #2E699E",
"a 	c #83A3BF", "b 	c #DDE5EE", "c 	c #F0F6F8", "d 	c #D3DBE4", "e 	c #8BA7BD",
"f 	c #10578A", "g 	c #6D97BD", "h 	c #B1C9DA", "i 	c #B0C7D9", "j 	c #B5C5D8",
"k 	c #AAC6DD", "l 	c #C0D0E3", "m 	c #B2C5DE", "n 	c #F7FDFF", "o 	c #FEFFFC",
"p 	c #90AFCC", "q 	c #BCD0DC", "r 	c #A5BDCE", "s 	c #ADBCCF", "t 	c #5984A9",
"u 	c #FEFCFF", "v 	c #FFFCFA", "w 	c #91B0CD", "x 	c #4577A1", "y 	c #E7EFF8",
"z 	c #EAEFF2", "A 	c #7E9EB9", "B 	c #1D5E91", "C 	c #FDFBFF", "D 	c #94B4D0",
"E 	c #407AA9", "F 	c #C6D2E0", "G 	c #E5EAED", "H 	c #85A5C1", "I 	c #B9CDD9",
"J 	c #06538B", "K 	c #80A0BB", "L 	c #91ACC3", "M 	c #8FAECB", "N 	c #90ABC2",
"O 	c #92ADC4", "P 	c #6590B6", "Q 	c #26598D", "R 	c #4A79A9", "S 	c #5485AF",
"T 	c #5980AC", "U 	c #97B7D3", "V 	c #87A7C3", "W 	c #F2F7F9", "X 	c #BDCCE0",
"Y 	c #EAF3FB", "Z 	c #5384AE", "` 	c #FAF7FC", " .	c #6A8EAF", "..	c #ECF1F4",
"+.	c #3B6E97", "@.	c #8AAAC6", "#.	c #88A8C4", "$.	c #F3F8FB", "%.	c #93B3CF",
"&.	c #A3BBCC", "*.	c #F5FAFD", "=.	c #9EBAD1", "-.	c #306291", ";.	c #316BA0",
">.	c #144E81", ",.	c #2F6190", "'.	c #E1EAF2", ").	c #C3CFDD", "!.	c #386999",
"~.	c #0C558D", "{.	c #6F99BF", "].	c #9FBBD2", "^.	c #99B9D6", "/.	c #FBFDFA",
"(.	c #A0BCD3", "_.	c #A5C0D7", ":.	c #97B2C9", "<.	c #94AFC6", "[.	c #4C789C",
"}.	c #205E98", "|.	c #628DB2", "1.	c #37669C", "2.	c #00457D", "3.	c #86A6C2",
"4.	c #DEE6EF", "5.	c #E2EBF3", "6.	c #E0E9F1", "7.	c #5580A5", "8.	c #C0CCDA",
"9.	c #BECDE1", "0.	c #C9D5E3", "a.	c #6F93B4", "b.	c #2C689C", "c.	c #7BA0C1",
"d.	c #C4D4E8", "e.	c #336594", "f.	c #195A94", "g.	c #B4C4D7", "h.	c #CFDBEA",
"i.	c #CCD8E6", "j.	c #CDD9E7", "k.	c #9CB8CF", "l.	c #769BBC", "m.	c #CBD7E5",
"n.	c #C0D4E1", "o.	c #6E92B3", "p.	c #0F568E", "q.	c #587FAB", "r.	c #1F5684",
"s.	c #B7CAD7", "t.	c #275A8E", "u.	c #E7EDEF", "v.	c #286599", "w.	c #A8C3DB",
"x.	c #EBF0F3", "y.	c #7F9FBA", "z.	c #D4DCE5", "A.	c #84A4C0", "B.	c #688CAD",
"C.	c #8BABC7", "D.	c #1B5B95", "E.	c #185993", "F.	c #4B7DA7", "G.	c #E9EEF0",
"H.	c #5C87AC", "I.	c #BBCADE", "J.	c #1D5C96", "K.	c #80A4C6", "L.	c #81A5C7",
"M.	c #2F5F95", "N.	c #F6FBFE", "O.	c #6C90B1", "P.	c #1F5D97", "Q.	c #25619B",
". . . . . . . . . . . + + + + + + + + + + + + @ # . . . . . $ $ ",
"% % % & . . . . . . . + * * * * * * * * * * * @ . . . . . = = = ",
"* * * * - ; ; ; ; ; = > > > > > > > > > > > > # ; ; ; ; $ * * * ",
", , , ' ) . . . . . ! ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ { . . . . . # ] ] ^ ",
"= / / # = ; ; ; ; . > ( ( ( ( ( ( ( ( ( ( ( @ ; ; ; ; & # ^ ^ _ ",
"> > ( ~ { . ; ; ; / * * * * * * * * * * * ( : ; ; ; . < [ [ * * ",
"' ' ^ ] ] - ; ; . } > > > > > > > > > > > | . ; ; ; . ] ^ ^ _ _ ",
"1 2 3 4 5 6 . . ; ~ 7 5 8 8 5 4 9 0 ~ ~ * a b c d d b e f / ] ] ",
"g h i j k l ; / / ( m n o o n b [ [ ( p j q r s t o o u v w ( 0 ",
"x 2 y z y 3 A ; B { C o o v n D * E F y y y 3 G H o o C u n I : ",
"J K L M N O P . Q R o v u o u S > T U w M e V t G o o o o v u O ",
"~ l 2 W 2 2 2 & ^ X n n o o Y ~ Z ` W 2 2 5 5  .o o n o v v n ..",
"% +.N @.#.e #.+.. $.o v u o j ( p %.w %.H H &.+.W u v o n *.=.% ",
"] < 2 5 8 5 2 b -.C u o C n ;.T 2 ` $.5 v 2  ./ >.,.'.o ).!.~.~.",
"~ [ {.=.].^.].p &././.o n c * P (._._.:.<.[.; ; ; }.> |.* 1.* ( ",
"2./ |.y G 1 G '.3./.u u u _.> w 1 4.5.6.b 7.; . . ) = ; = ; & ) ",
"@ ] ! 8.9.).l 0.a.../.o o 9 b.c.d.d.d.6 8.e.; ; . / f.Q B Q ! Q ",
"b.> > g.h.i.j.h.m k.o o Y ( > l.m.m.0.n.F o.. . ; p.> q.> 0 | * ",
"; ; . 9 z 4.1 G 4 [.u o X > ~ R 6.1 '.G G N a.r./ >.G o s.f - ; ",
"t.Q B } g.i k j m l.G o l.( [ > ^.j s &.].e v *.u.5 n C /.5 U @ ",
"~ v.[ [ w.Y G y y x.y.C E * > ~ q.Y y x.8.z.o o v n v o v n v y ",
"; . . . -.M e e A.p [.d | ~ * ( [ B.C.<.x 5 o o v o o u v u v y.",
"D.E.f.f.F.G.c 2 2 5 h./ f * * > > >.d 5 H.u u o o o o v o o I.J.",
"{ { { ( [ K.%.w w p %.) # > b.* f ; ; r.m.o o o u u n o W L.M.~ ",
"; . ; . . L N.5 v n v e - * * * , ; ; ; r.#.u.*.o $.4.O.= . . ; ",
"t.M.D.D.P.M.J.J.M.D.P.! ; f.( ( . ; . . ; ; ; . . . . . # P.P.P.",
"Q.}.< < M.M.< < P.M.M.Q.. >.> P.. ; ; ; . . . ; ; . . ; . B M.M.",
". . ; . ; ; ; . . . ; ; ; ) v., ; ; . ; ; ; . . ; ; . . ; . . . ",
"< M.M.< }.M.Q.M.}.Q.M.M.@ . f./ ; ; ; . . . ; ; ; ; ; ; ; ; & M.",
"J.}.J.B }.t.B < B }.}.M.t.. % ; ; ; ; ; ; ; ; ; ; . ; ; ; . . # ",
". . . ; . - ; ; ; ; ; ; . ; ; ; ; ; ; ; ; ; ; ; ; ; ; ; ; . ; . ",
"< M.{ < { | < + < < Q.+ { E.; ; ; ; ; ; ; ; ; ; ; ; ; ; ; ; . . "]

# For GUI-only mode redirect console output to NULL and errors to Tmpfile
if os.path.basename(sys.executable).startswith("pythonw"):
	sys.stdout =  open(os.devnull, "w")
	sys.stderr =  open(os.path.join(tempfile.gettempdir(), "stderr-" + os.path.basename(sys.argv[0])), "w")




class  Backend(object):


    def  __init__(self):
        self.com = self.Com(self)


    class Com(object):
        img      = None  # None iff not yet written
        
        img_nr_from    = 0
        img_nr_to      = 0
        img_timer_from = 0
        img_timer_to   = 0
        img_per_second = 0

        sock     = None  # None iff disconnected.
        sock_to  = None

        srv_ip   = 'ccimx8x-sbc-pro'
        srv_port = 2002

        x0       = 0
        y0       = 0
        dx       = 1920
        dy       = 1080
        incrx    = 1
        incry    = 1


        def  __init__(self, backend):
            self.backend = backend


        def  __del__(self):
            self.disconnect()
            

        def connect(self, ip, port):

            if((None != self.sock) and (ip==self.sock_to[0]) and (port==self.sock_to[1])):
                return True

            if(None != self.sock):
                self.disconnect()
                
            self.sock =  socket.socket(socket.AF_INET, socket.SOCK_STREAM)
            self.sock_to = (ip, port)

            logging.info('connecting to %s port %s' % self.sock_to)
            try:
                self.sock.settimeout(0.75)
                self.sock.connect(self.sock_to)
                self.sock.settimeout(None)
            except Exception:
                self.sock.settimeout(None)
                #print('something\'s wrong with %s:%d. Exception type is %s' % (self.sock_to[0], self.sock_to[1], `e`))
                #return False
                raise
            
            self.img_nr = 0
            
            return True


        def disconnect(self):

            if(None != self.sock):
                logging.info('disconnecting from %s port %s' % self.sock_to)
                self.sock.close()

            self.sock    = None
            self.sock_to = None


        def receive_image(self):
            
            if 0 == self.img_nr_from:
                self.img_nr_from    = 1
                self.img_timer_from = time.time()
                self.img_nr_to      = 1
                

            if(None == self.sock):
                return False

            # Construct and send the request header

            logging.debug('requesting image size: ([x0,y0],[dx,dy],[incrx,incry]):([%4d,%4d],[%4d,%4d],[%2d,%2d])'
                           % (self.x0, self.y0, self.dx, self.dy, self.incrx, self.incry))
            hdr_req = struct.pack("IIIIII", self.x0, self.y0, self.dx, self.dy, self.incrx, self.incry)
            #s = binascii.hexlify(hdr_req) ; print s
            try:
                self.sock.sendall(hdr_req)
            except Exception:
                raise


            # Receive and interpret the response header
            
            data = bytearray()

            while len(data) < 28:
                try:
                    data += self.sock.recv(28 - len(data))
                except Exception:
                    raise

            hdr_resp  =  struct.unpack_from("IIIIIII", buffer(data))
            img_bytes = hdr_resp[0] - 28
            (img_x0, img_y0, img_dx, img_dy, img_incrx, img_incry) = hdr_resp[1:7]
            #print binascii.hexlify(data)

            if img_bytes < 0:
                raise

            # Receive image data based on response header information

            logging.debug('receiving  image size: ([x0,y0],[dx,dy],[incrx,incry]):([%4d,%4d],[%4d,%4d],[%2d,%2d])'
                           % (img_x0, img_y0, img_dx, img_dy, img_incrx, img_incry))
            print('receiving  image size: ([x0,y0],[dx,dy],[incrx,incry]):([%4d,%4d],[%4d,%4d],[%2d,%2d])'
                           % (img_x0, img_y0, img_dx, img_dy, img_incrx, img_incry))

            #img_data = bytearray()
            #while len(img_data) < img_bytes:
            #    try:
            #        #img_data += self.sock.recv(img_bytes - len(img_data))
            #        img_data.extend(self.sock.recv(img_bytes - len(img_data)))  #seems to slightly improve performance
            #        logging.debug('Received  %10d of %10d Bytes. ( %10d to go )' % (len(img_data), img_bytes, img_bytes - len(img_data)))
            #    except Exception:
            #        raise
            #
            ##The following version does not use an internal interim string representation - may be faster
            img_data = bytearray(img_bytes)
            mv_img_data = memoryview(img_data)
            len_img_data = 0
            while len_img_data < img_bytes:
                bytesRead = self.sock.recv_into(mv_img_data, img_bytes - len_img_data)
                mv_img_data = mv_img_data[bytesRead:]
                len_img_data += bytesRead

            self.img = dict(x0=img_x0,y0=img_y0,dx=img_dx,dy=img_dy,incrx=img_incrx,incry=img_incry,data=img_data)
            
            #Todo: Overflow Handling
            self.img_timer_to    = time.time()
            self.img_nr_to      += 1
            
            self.img_per_second = (self.img_nr_to - self.img_nr_from) / (self.img_timer_to - self.img_timer_from)  # > 0
            #print "%u  ( %u - %u )/( %u - %u ) " %  (self.img_per_second, self.img_nr_to, self.img_nr_from, self.img_timer_to, self.img_timer_from)



        def write_png(self, path, chk_rgb_true):

            if(None == self.img):
                return False

            directory = os.path.dirname(path)
            if not os.path.exists(directory):
                os.makedirs(directory)

            logging.info('writing image to file %s' % path)
            
            # pgm (problematic on some windows machines)
            #
            #img_file = open(path, 'w', 0)
            #img_file.write("P5" + " " + str(self.img['dx']) + " " + str(self.img['dy']) + " 255 ")
            #img_file.write(self.img['data'])
            #img_file.close()
 
            dx    = int(self.img['dx'])/int(self.img['incrx'])
            dy    = int(self.img['dy'])/int(self.img['incry'])
            pitch = dx

            mv_img_data = memoryview(self.img['data'])
            if(chk_rgb_true):
                dy /= 3
                img_data_packed = bytearray(chain.from_iterable(izip(mv_img_data[2 * pitch * dy:], mv_img_data[1 * pitch * dy:], mv_img_data[0 * pitch * dy:])))
            else:
                dy /= 1
                img_data_packed = bytearray(chain.from_iterable(izip(mv_img_data[0 * pitch * dy:], mv_img_data[0 * pitch * dy:], mv_img_data[0 * pitch * dy:])))
            cpb = gtk.gdk.pixbuf_new_from_data(buffer(img_data_packed), gtk.gdk.COLORSPACE_RGB, False, 8, dx, dy, 3*pitch)
            cpb.save(path, "png")
            
    











class  Frontend(object):

    def  __init__(self, arg_img_access):
        self.dialog  = self.Dialog()
        self.winMain = self.WinMain(arg_img_access, self.dialog)


    class  WinMain(gtk.VBox):
        """
            +----------------------------------------------------+
            |  MenuBar                                           |
            +----------------------------------------------------+
            |                       .                            |
            |                       .                            |
            |                    Content                         |
            |     Settings          .          Display           |
            |                       .                            |
            |                       .                            |
            +----------------------------------------------------+
            |  StatusBar                                         |
            +----------------------------------------------------+
        """

        window = None


        ###########################################################

        def  __init__(self, arg_img_access, dialog):
            
            self.img_access = arg_img_access
            appicon         = gtk.gdk.pixbuf_new_from_xpm_data(app_icon)

            self.window =  gtk.Window(gtk.WINDOW_TOPLEVEL)
            self.window.resize(1020,600)
            self.window.connect("destroy",      lambda   a: gtk.main_quit())
            self.window.connect("delete_event", lambda a,b: gtk.main_quit())
            self.window.set_title("VCImgNetClient")
            self.window.set_icon(appicon)


            vbox = gtk.VBox()

            self.menu_bar = self.MenuBar(self, dialog)
            vbox.pack_start(self.menu_bar, False, False)

            self.content = self.Content(self)
            vbox.pack_start(self.content, True, True)

            self.status_bar = self.StatusBar(self)
            vbox.pack_start(self.status_bar, False, False)

            vbox.show()


            self.window.add(vbox)
            self.window.show()


        class  Content(gtk.HBox):
            """ The Main Window Content """

            def  __init__(self, winMain):

                self.winMain = winMain

                gtk.HBox.__init__(self)

                self.settings = self.Settings(winMain)
                self.pack_start(self.settings, False, True)

                self.display = self.Display(winMain)
                self.pack_start(self.display, True, True)

                self.show()


            class  Settings(gtk.VBox):

                def  __init__(self, winMain):

                    self.winMain = winMain

                    gtk.VBox.__init__(self)
                    self.set_border_width(10)

                    self.tbl =  gtk.Table(3,5,False)
                    # IP
                    self.lbl_ip = gtk.Label("Server IP")
                    self.tbl.attach(self.lbl_ip, 0,1, 0,1, gtk.EXPAND | gtk.SHRINK | gtk.FILL,  gtk.FILL, 0, 0)
                    self.lbl_ip.show()
                    self.entry_ip = gtk.Entry()
                    self.tbl.attach(self.entry_ip, 1,3, 0,1, gtk.EXPAND | gtk.SHRINK | gtk.FILL,  gtk.FILL, 0, 0)
                    self.entry_ip.show()
                    # Port
                    self.lbl_port = gtk.Label("Server Port")
                    self.tbl.attach(self.lbl_port, 0,1, 1,2, gtk.EXPAND | gtk.SHRINK | gtk.FILL,  gtk.FILL, 0, 0)
                    self.lbl_port.show()
                    self.adj_port = gtk.Adjustment(value=2002, lower=0, upper=64000, step_incr=1, page_incr=100, page_size=0)
                    self.entry_port = gtk.SpinButton(self.adj_port, climb_rate=1, digits=0)
                    self.tbl.attach(self.entry_port, 1,3, 1,2, gtk.EXPAND | gtk.SHRINK | gtk.FILL,  gtk.FILL, 0, 0)
                    self.entry_port.show()
                    # x0, y0
                    self.lbl_x0y0 = gtk.Label("[x0, y0]")
                    self.tbl.attach(self.lbl_x0y0, 0,1, 2,3, gtk.EXPAND | gtk.SHRINK | gtk.FILL,  gtk.FILL, 0, 0)
                    self.lbl_x0y0.show()
                    self.adj_x0 = gtk.Adjustment(value=0, lower=0, upper=5200, step_incr=1, page_incr=100, page_size=0)
                    self.entry_x0 = gtk.SpinButton(self.adj_x0, climb_rate=1, digits=0)
                    self.tbl.attach(self.entry_x0, 1,2, 2,3, gtk.EXPAND | gtk.SHRINK | gtk.FILL,  gtk.FILL, 0, 0)
                    self.entry_x0.show()
                    self.adj_y0 = gtk.Adjustment(value=0, lower=0, upper=5200, step_incr=1, page_incr=100, page_size=0)
                    self.entry_y0 = gtk.SpinButton(self.adj_y0, climb_rate=1, digits=0)
                    self.tbl.attach(self.entry_y0, 2,3, 2,3, gtk.EXPAND | gtk.SHRINK | gtk.FILL,  gtk.FILL, 0, 0)
                    self.entry_y0.show()
                    # dx, dy
                    self.lbl_dxdy = gtk.Label("[dx, dy]")
                    self.tbl.attach(self.lbl_dxdy, 0,1, 3,4, gtk.EXPAND | gtk.SHRINK | gtk.FILL,  gtk.FILL, 0, 0)
                    self.lbl_dxdy.show()
                    self.adj_dx = gtk.Adjustment(value=0, lower=0, upper=5200, step_incr=1, page_incr=100, page_size=0)
                    self.entry_dx = gtk.SpinButton(self.adj_dx, climb_rate=1, digits=0)
                    self.tbl.attach(self.entry_dx, 1,2, 3,4, gtk.EXPAND | gtk.SHRINK | gtk.FILL,  gtk.FILL, 0, 0)
                    self.entry_dx.show()
                    self.adj_dy = gtk.Adjustment(value=0, lower=0, upper=5200, step_incr=1, page_incr=100, page_size=0)
                    self.entry_dy = gtk.SpinButton(self.adj_dy, climb_rate=1, digits=0)
                    self.tbl.attach(self.entry_dy, 2,3, 3,4, gtk.EXPAND | gtk.SHRINK | gtk.FILL,  gtk.FILL, 0, 0)
                    self.entry_dy.show()
                    # incrx, incry
                    self.lbl_incrxincry = gtk.Label("[incrx, incry]")
                    self.tbl.attach(self.lbl_incrxincry, 0,1, 4,5, gtk.EXPAND | gtk.SHRINK | gtk.FILL,  gtk.FILL, 0, 0)
                    self.lbl_incrxincry.show()
                    self.adj_incrx = gtk.Adjustment(value=1, lower=1, upper=10, step_incr=1, page_incr=1, page_size=0)
                    self.entry_incrx = gtk.SpinButton(self.adj_incrx, climb_rate=1, digits=0)
                    self.tbl.attach(self.entry_incrx, 1,2, 4,5, gtk.EXPAND | gtk.SHRINK | gtk.FILL,  gtk.FILL, 0, 0)
                    self.entry_incrx.show()
                    self.adj_incry = gtk.Adjustment(value=1, lower=1, upper=10, step_incr=1, page_incr=1, page_size=0)
                    self.entry_incry = gtk.SpinButton(self.adj_incry, climb_rate=1, digits=0)
                    self.tbl.attach(self.entry_incry, 2,3, 4,5, gtk.EXPAND | gtk.SHRINK | gtk.FILL,  gtk.FILL, 0, 0)
                    self.entry_incry.show()

                    self.pack_start(self.tbl, False, True)
                    self.tbl.show()

                    self.chk_continuous = gtk.CheckButton("Receive Continuously")
                    self.pack_start(self.chk_continuous, False, True)
                    self.chk_continuous.show()



                    self.btn_how_select_grey = gtk.RadioButton(None,"Interpret as Grey")
                    self.pack_start(self.btn_how_select_grey, False, True)
                    self.btn_how_select_grey.show()
                    
                    self.btn_how_select_flscol = gtk.RadioButton(self.btn_how_select_grey,"Interpret as False Color")
                    self.pack_start(self.btn_how_select_flscol, False, True)
                    self.btn_how_select_flscol.show()

                    self.btn_how_select_rgb = gtk.RadioButton(self.btn_how_select_grey,"Interpret as RGB")
                    self.pack_start(self.btn_how_select_rgb, False, True)
                    self.btn_how_select_rgb.show()

                    self.btn_receive_img = gtk.ToggleButton("Receive Image")
                    self.pack_start(self.btn_receive_img, False, True)
                    self.btn_receive_img.show()



                    # Image Path
                    self.chk_imgStoreContinuous = gtk.CheckButton("Store Images Continuously (Big Data!)")
                    self.pack_end(self.chk_imgStoreContinuous, False, True)
                    self.chk_imgStoreContinuous.show()
                    hbox = gtk.HBox()
                    self.lbl_imagePath = gtk.Label("Image Path")
                    hbox.pack_start(self.lbl_imagePath, True, True)
                    self.lbl_imagePath.show()
                    self.entry_imagePath = gtk.Entry()
                    hbox.pack_start(self.entry_imagePath, False, True)
                    self.entry_imagePath.show()
                    self.pack_end(hbox, False, True)
                    hbox.show()
                    sep = gtk.HSeparator()
                    self.pack_end(sep, False, True)
                    sep.show()

                    self.btn_store_img = gtk.Button("Store Current Image")
                    self.pack_end(self.btn_store_img, False, True)
                    self.btn_store_img.show()
                    
                    self.show()


            class  Display(gtk.Table):

                def  __init__(self, winMain):

                    self.winMain = winMain

                    gtk.Table.__init__(self, 3,3,False)


                    # Ruler horizontal/vertical
                    self.hruler = gtk.HRuler()
                    self.hruler.set_metric(gtk.PIXELS)
                    self.hruler.set_range(0, 5200, 0, 5200)
                    
                    self.vruler = gtk.VRuler()
                    self.vruler.set_metric(gtk.PIXELS)
                    self.vruler.set_range(0, 5200, 0, 5200)
                    
                    # Scrolled Window containing drawing Area
                    self.area = gtk.DrawingArea()
                    self.area.set_events(gtk.gdk.POINTER_MOTION_MASK | gtk.gdk.POINTER_MOTION_HINT_MASK )
                    self.area.set_size_request(5200, 5200)
                    self.area.connect("expose-event", self.area_expose_cb)
                    
                    self.sw = gtk.ScrolledWindow()
                    self.sw.add_with_viewport(self.area)
                    
                    self.attach(self.hruler, 1,2, 0,1, gtk.EXPAND | gtk.SHRINK | gtk.FILL,  gtk.FILL, 0, 0)
                    self.attach(self.vruler, 0,1, 1,2, gtk.FILL,  gtk.EXPAND | gtk.SHRINK | gtk.FILL, 0, 0)
                    self.attach(self.sw,     1,2, 1,2, gtk.FILL, gtk.FILL, 0,0)
                    
                    # Tell mouse motion to rulers for updating the mouse position marker.
                    def motion_notify(ruler, event):
                        return ruler.emit("motion_notify_event", event)
                    
                    self.sw.connect_object("motion_notify_event", motion_notify, self.hruler)
                    self.sw.connect_object("motion_notify_event", motion_notify, self.vruler)
                    
                    # Set ruler scaling regarding to the visible area
                    def size_allocate_cb(wid, allocation):
                        x, y, w, h = allocation
                        #print("x: %d   y: %d   w:%d   h:%d" % (x,y,w,h))
                        lower,upper,pos,maxSize = self.hruler.get_range()
                        maxSize = max(maxSize, w)
                        self.hruler.set_range(lower, lower+w, pos, maxSize)
                        lower,upper,pos,maxSize = self.vruler.get_range()
                        maxSize = max(maxSize, h)
                        self.vruler.set_range(lower, lower+h, pos, maxSize)
                    
                    self.sw.connect('size-allocate', size_allocate_cb)

                    # Recalculate ruler ranges with respect to scroll window and mouse position
                    def val_cb(adj, ruler, horiz):
                        #print("lower: %d   value: %d   upper-page_size:%d      upper:%d, page_size:%d" % (adj.lower, adj.value, adj.upper-adj.page_size,adj.upper,adj.page_size))
                        #position of the scrollbar between 'lower' and 'upper -page_size'.
                        #lower is 0, upper is the maximum value of the scroll region (self.are.set_size_request()), page_size is the scroll knob length.
                        v = adj.value 
                        if horiz:
                            span = self.sw.get_allocation().width
                        else:
                            span = self.sw.get_allocation().height
                        lower,upper,pos,maxSize = ruler.get_range()
                        ruler.set_range(v, v+span, pos-v, maxSize)
                        while gtk.events_pending():
                            gtk.main_iteration()

                    self.hadj = self.sw.get_hadjustment()
                    self.hadj.connect('value-changed', val_cb, self.hruler, True)
                    self.vadj = self.sw.get_vadjustment()
                    self.vadj.connect('value-changed', val_cb, self.vruler, False)
                    
                    self.hruler.show()
                    self.vruler.show()
                    self.sw.show()
                    self.area.show()

                    self.show()
 

                def area_expose_cb(self, area, event):
                    
                    if(None==self.winMain.img_access):
                        return True
                    
                    img   = self.winMain.img_access()

                    if(None==img):
                        return True

                    style = self.area.get_style()
                    gc    = style.fg_gc[gtk.STATE_NORMAL]

                    # would work, but it's slow.
                    #self.area.set_size_request(int(img['dx'])/int(img['incrx']),int(img['dy'])/int(img['incry']))

                    dx    = int(img['dx'])/int(img['incrx'])
                    dy    = int(img['dy'])/int(img['incry'])
                    pitch = dx


                    # locale variables are faster than global!
                    
                    afalsecolor_r = [255, 5, 11, 17, 23, 29, 35, 41, 47, 53, 59, 65, 71, 77, 83, 89, 95, 101, 107, 113, 119, 125, 125, 131, 137, 143, 149, 155, 161, 167, 173, 179, 191, 197, 203, 209, 215, 221, 227, 233, 239, 245, 251, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 250, 244, 238, 232, 226, 220, 214, 208, 202, 196, 190, 184, 178, 172, 166, 160, 154, 148, 142, 136, 130, 124, 118, 112, 106, 100, 94, 88, 82, 76, 70, 64, 58, 52, 46, 40, 34, 28, 22, 16, 10, 4, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 2, 8, 14, 20, 26, 32, 38, 44, 50, 56, 62, 68, 74, 80, 86, 92, 98, 104, 110, 116, 122, 128, 134, 140, 146, 152, 158, 164, 170, 176, 182, 188, 194, 200, 206, 212, 218, 224, 230, 236, 242, 248, 255]

                    afalsecolor_g = [255, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 2, 8, 14, 20, 26, 32, 38, 44, 50, 56, 62, 68, 74, 80, 86, 92, 98, 104, 110, 116, 122, 128, 134, 140, 146, 152, 158, 164, 170, 176, 182, 188, 194, 200, 206, 212, 218, 224, 230, 236, 242, 248, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 250, 244, 238, 232, 226, 220, 214, 208, 202, 196, 190, 184, 178, 172, 166, 160, 154, 148, 142, 136, 130, 124, 118, 112, 106, 100, 94, 88, 82, 76, 70, 64, 58, 52, 46, 40, 34, 28, 22, 16, 10, 4, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0]

                    afalsecolor_b = [255,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  2,  8, 14, 20, 26, 32, 38, 44, 50, 56, 62, 68, 74, 80, 86, 92, 98,104,110,116,122,128,134,140,146,152,158,164,170,176,182,188,194,200,206,212,218,224,230,236,242,248,254,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255]


                    if('rgb'==self.how_select):
                        dy /= 3
                        mv_img_data = memoryview(img['data'])
                        img_data_packed = bytearray(chain.from_iterable(izip(mv_img_data[0 * pitch * dy:], mv_img_data[1 * pitch * dy:], mv_img_data[2 * pitch * dy:])))
                        self.area.window.draw_rgb_image(gc, 0,0, dx,dy, gtk.gdk.RGB_DITHER_NONE, buffer(img_data_packed),3*pitch,0,0)
                    else:
                        if('flscol'==self.how_select):
                            
                            mv_img_data = memoryview(img['data']).tolist()

                            start_time = time.time()
                            img_data_packed = bytearray()
                            for y in range(0,dy):
                                img_data_packed.extend([lut[x] for x in mv_img_data[(y+0) * pitch + (   0): (y+0) * pitch + (  dx)] for lut in (afalsecolor_r, afalsecolor_g, afalsecolor_b)])
                            elapsed_time = time.time() - start_time
                            #print "timeNewLut: " + str(elapsed_time)
                            
                            # Example, Only left half color lut
                            #
                            #start_time = time.time()
                            #img_data_packed = bytearray()
                            #for y in range(0,dy):
                            #    img_data_packed.extend([lut[x] for x in mv_img_data[(y+0) * pitch + (   0): (y+0) * pitch + (dx/2)] for lut in (afalsecolor_r, afalsecolor_g, afalsecolor_b)])
                            #    img_data_packed.extend([    x  for x in mv_img_data[(y+0) * pitch + (dx/2): (y+0) * pitch + (dx  )] for lut in (afalsecolor_r, afalsecolor_g, afalsecolor_b)])
                            #elapsed_time = time.time() - start_time
                            #print "timeHALFLUT: " + str(elapsed_time)
                            
                            self.area.window.draw_rgb_image(gc, 0,0, dx,dy, gtk.gdk.RGB_DITHER_NONE, buffer(img_data_packed),3*pitch,0,0)
                        else: #grey
                            self.area.window.draw_gray_image(gc, 0,0, 
                                int(img['dx'])/int(img['incrx']),int(img['dy'])/int(img['incry']), 
                                gtk.gdk.RGB_DITHER_NONE, buffer(img['data']), int(img['dx'])/int(img['incrx']))

                    return True


        class  MenuBar(gtk.MenuBar):

            def  __init__(self, winMain, dialog):

                self.winMain = winMain
                self.dialog = dialog

                gtk.MenuBar.__init__(self)

                self.append(self.menu_file())
                self.append(self.menu_help())

                self.show()


            def  menu_file(self):

                mfQuit =  gtk.MenuItem("Quit")
                mfQuit.connect_object("activate", lambda a: gtk.main_quit(), "mfQuit")
                mfQuit.show()


                mf =  gtk.Menu()
                mf.append(mfQuit)
                mf.show()

                bmf = gtk.MenuItem("File")
                bmf.set_submenu(mf)
                bmf.show()

                return bmf


            def  menu_help(self):

                mhAbout =  gtk.MenuItem("About")
                mhAbout.connect("activate", self.dialog.About, "mhAbout")
                mhAbout.show()

                mh =  gtk.Menu()
                mh.append(mhAbout)
                mh.show()

                bmh = gtk.MenuItem("Help")
                bmh.set_submenu(mh)
                bmh.show()

                return bmh


        
        class  StatusBar(gtk.Statusbar):

            def  __init__(self, winMain):

                self.winMain = winMain

                gtk.Statusbar.__init__(self)
                self.show()


    class  Dialog(object):

        def  StoreImageFilename(self):
            
            filename = None

            storewindow = gtk.FileChooserDialog(title="Storing Image: Target", action=gtk.FILE_CHOOSER_ACTION_SAVE, buttons=(gtk.STOCK_CANCEL,gtk.RESPONSE_CANCEL,gtk.STOCK_SAVE,gtk.RESPONSE_OK))
            filter = gtk.FileFilter()
            filter.set_name("PNG")
            filter.add_mime_type("image/png")
            filter.add_pattern("*.png")
            storewindow.add_filter(filter)
            storewindow.set_current_name("image.png")
		    
            response = storewindow.run()
            if response == gtk.RESPONSE_OK:
                #print storewindow.get_filename(), 'selected'
                filename = storewindow.get_filename()
            #elif response == gtk.RESPONSE_CANCEL:
            #    print 'Closed, no files selected'
            storewindow.destroy()
            
            return filename


        def  About(self, widget, string):
            appicon         = gtk.gdk.pixbuf_new_from_xpm_data(app_icon)
            
            about = gtk.AboutDialog()
            about.set_name("VCImgNetClient")
            about.set_copyright("(C) 2015-2016 by Vision Components")
            about.connect("response", lambda self, r: self.destroy())
            about.set_icon(appicon)
            about.show()
            return True




class  Main(Backend, Frontend):
    """ 
        Methods in the main class connect and define all actions regarding
        the graphical user interface and the backend.
    """

    def  __init__(self):
        Backend.__init__(self)
        Frontend.__init__(self, self.img_access)

        self.winMain.content.settings.btn_receive_img.connect("toggled", self.do_receive_img_start, "btn_receive_img")
        self.winMain.content.settings.btn_store_img.connect("clicked", self.do_store_img, "btn_store_img")
        self.winMain.content.settings.btn_store_img.set_sensitive(False)
        self.winMain.content.settings.entry_ip.set_text(   str(self.com.srv_ip  ))
        self.winMain.content.settings.entry_port.set_text( str(self.com.srv_port))
        self.winMain.content.settings.entry_x0.set_text(   str(self.com.x0      ))
        self.winMain.content.settings.entry_y0.set_text(   str(self.com.y0      ))
        self.winMain.content.settings.entry_dx.set_text(   str(self.com.dx      ))
        self.winMain.content.settings.entry_dy.set_text(   str(self.com.dy      ))
        self.winMain.content.settings.entry_incrx.set_text(str(self.com.incrx   ))
        self.winMain.content.settings.entry_incry.set_text(str(self.com.incry   ))
        self.winMain.content.display.how_select = 'grey'
        self.winMain.content.settings.chk_continuous.set_active(True)
        self.winMain.content.settings.btn_how_select_grey.connect(  "toggled", self.tag_select_toggler_callback, 'grey'  )
        self.winMain.content.settings.btn_how_select_flscol.connect("toggled", self.tag_select_toggler_callback, 'flscol')
        self.winMain.content.settings.btn_how_select_rgb.connect(   "toggled", self.tag_select_toggler_callback, 'rgb'   )
        self.winMain.content.settings.entry_imagePath.set_text(tempfile.gettempdir())
        self.winMain.content.settings.chk_imgStoreContinuous.set_active(False)

    # Enables the Gui Window to update the Image Content Display asynchronously (e.g. by scrolling over the Image)
    def  img_access(self):
        return self.com.img

    def tag_select_toggler_callback(self, widget, data=None):
        #print "%s was toggled %s." % (data, ("OFF", "ON")[widget.get_active()])
        if(True==widget.get_active()):
            self.winMain.content.display.how_select = data

    def  do_receive_img_start(self, widget=None, data=None):
        if(self.winMain.content.settings.btn_receive_img.get_active()):
            self.winMain.content.display.area.window.clear()
        self.winMain.content.settings.btn_store_img.set_sensitive(False)
        self.do_receive_img(widget, data)

    def  do_receive_img(self, widget=None, data=None):
        if(self.winMain.content.settings.btn_receive_img.get_active()):
            self.com.srv_ip   = str(self.winMain.content.settings.entry_ip.get_text())
            self.com.srv_port = int(self.winMain.content.settings.entry_port.get_text())
            self.com.x0       = int(self.winMain.content.settings.entry_x0.get_text())
            self.com.y0       = int(self.winMain.content.settings.entry_y0.get_text())
            self.com.dx       = int(self.winMain.content.settings.entry_dx.get_text())
            self.com.dy       = int(self.winMain.content.settings.entry_dy.get_text())
            self.com.incrx    = int(self.winMain.content.settings.entry_incrx.get_text())
            self.com.incry    = int(self.winMain.content.settings.entry_incry.get_text())
            try:
                self.com.connect(self.com.srv_ip, self.com.srv_port)
                self.com.receive_image()
            except Exception, e:
                self.com.disconnect()
                msgwindow = gtk.MessageDialog(parent=None, flags=0, type=gtk.MESSAGE_ERROR, buttons=gtk.BUTTONS_CLOSE, message_format=None)
                msgwindow.set_markup("Failed to connect to\nIP %s  Port %d\n%s" % (self.com.srv_ip, self.com.srv_port, e))
                msgwindow.run()
                msgwindow.destroy()
                self.winMain.content.settings.btn_receive_img.set_active(False)
                return False

            if(self.winMain.content.settings.chk_imgStoreContinuous.get_active()):
                imgFilename =  os.path.join(self.winMain.content.settings.entry_imagePath.get_text(), os.path.splitext(__file__)[0], "{}.png".format(datetime.datetime.now().strftime("%y%m%dT%H%M%S%f")))
                self.com.write_png(imgFilename, 'rgb'==self.winMain.content.display.how_select)

            self.winMain.content.settings.entry_x0.set_text(   str(self.com.img[   'x0']))
            self.winMain.content.settings.entry_y0.set_text(   str(self.com.img[   'y0']))
            self.winMain.content.settings.entry_dx.set_text(   str(self.com.img[   'dx']))
            self.winMain.content.settings.entry_dy.set_text(   str(self.com.img[   'dy']))
            self.winMain.content.settings.entry_incrx.set_text(str(self.com.img['incrx']))
            self.winMain.content.settings.entry_incry.set_text(str(self.com.img['incry']))
            self.winMain.status_bar.pop( self.winMain.status_bar.get_context_id("img_per_second"))
            self.winMain.status_bar.push(self.winMain.status_bar.get_context_id("img_per_second"), "Avg transferred fps: %u" % self.com.img_per_second)
            self.winMain.content.display.area_expose_cb(self.winMain.content.display.area, None)
            while gtk.events_pending():
                gtk.main_iteration(False)
            if(self.winMain.content.settings.chk_continuous.get_active()):
                 gobject.idle_add(self.do_receive_img)
            else:
                self.com.disconnect()
                self.winMain.content.settings.btn_receive_img.set_active(False)
            if not(self.winMain.content.settings.btn_receive_img.get_active()):
                self.winMain.content.settings.btn_store_img.set_sensitive(True)
        else:
            self.com.disconnect()
        return False #important for removing from idle calls to execute it therin only once (gobject.idle_add() sets this function to be executed always if gtk is idle; the function is removed from the idle loop, if it returns false).


    def  do_store_img(self, widget=None, data=None):
        filename = self.dialog.StoreImageFilename()
        if None != filename:
            #print filename, 'selected.'
            self.com.write_png(filename, 'rgb'==self.winMain.content.display.how_select)
        #else:
        #    print 'No valid filename, or user interrupt'


#import profile
#profile.run('Main();gtk.main()')

################################################################################
if __name__=="__main__":
    Main()
    gtk.main() # GUI Loop



