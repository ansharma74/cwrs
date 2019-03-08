#!/usr/bin/env python
# -*- coding: utf-8 -*-
# Copyright (C) 2008 洪任諭 Hong Jen Yee (PCMan) <pcman.tw@gmail.com>
# Released under GNU General Public License

import pygtk
pygtk.require("2.0")
import gtk, gtk.glade,gobject
import os, signal
import ConfigParser
import gettext
_ = gettext.gettext
import globals

ACTION_SEND=0
ACTION_RECV=1

gettext.bindtextdomain('gtknetcat', globals.locale_dir)
gettext.textdomain('gtknetcat')

# Due to bugs of Glade, titles are not automatically translated
page_titles=[_('File Transmission'), _('Send Files'), _('Receive Files')]

class Wizard:
    def __init__(self):
        self.config = ConfigParser.SafeConfigParser()
        self.config.add_section('history')
        self.config.read(os.path.expanduser('~/.config/gtknetcat/config'))

        file= '%s/wizard.glade' % globals.ui_dir
        self.xml = xml = gtk.glade.XML(file, domain='gtknetcat')
        xml.signal_autoconnect( self )
        self.wiz = w = xml.get_widget('wizard')
        self.action = ACTION_SEND
        p1=xml.get_widget('page1')
        w.set_page_complete(p1, True)
        self.send_page = xml.get_widget('page2_send')
        self.send_list = None
        self.recv_page = xml.get_widget('page2_recv')
        self.recv_page.hide()
        self.progress_page = xml.get_widget('page3')
        self.pid = 0
        self.stdi = self.stdo = self.stde = 0
        self.progress = xml.get_widget('progress_output').get_buffer()
        self.progress_scroll = xml.get_widget('progress_scroll')
        w.show()

    def on_send_toggled( self, btn ):
        if btn.get_active():
            self.action=ACTION_SEND
            self.send_page.show()
        else:
            self.send_page.hide()

    def on_recv_toggled( self, btn ):
        if btn.get_active():
            self.action=ACTION_RECV
            self.recv_page.show()
        else:
            self.recv_page.hide()

    # before show page
    def on_prepare( self, wiz, page ):
        cur_page = wiz.get_current_page()
        # Due to bugs of Glade, titles are not automatically translated
        if cur_page < len(page_titles):
            wiz.set_page_title(page, page_titles[cur_page])

        if page == self.progress_page:
            self.perform_action()
        elif page == self.send_page:
            self.init_send()

            try:
                p = self.config.get('history', 'dest_port')
            except:
                p = '9000'
            self.xml.get_widget('dest_port').set_text(p)

            try:
                h = self.config.get('history', 'dest_host')
                self.xml.get_widget('dest_host').set_text(h)
            except:
                pass
        else:
            if page == self.recv_page:
                try:
                    p = self.config.get('history', 'listen_port')
                except:
                    p = '9000'
                self.xml.get_widget('listen_port').set_text(p)

                try:
                    d = self.config.get('history', 'recv_dir')
                except:
                    d = os.path.expanduser('~')
                self.xml.get_widget('recv_dir').set_filename( d )

            wiz.set_page_complete(page, True)

    # when there are text in stdout and stderr of netcat
    def on_io(self, fd, cond):
        # print fd, cond
        if cond & gobject.IO_IN:
            line = os.read( fd, 1024 )
            it = self.progress.get_end_iter()
            self.progress.insert(it, line)
        if cond & gobject.IO_ERR:
            self.wiz.set_page_complete( self.progress_page, True )
            return False
        if cond & gobject.IO_HUP:
            self.wiz.set_page_complete( self.progress_page, True )
            return False

        adj = self.progress_scroll.get_vadjustment()
        adj.set_value( adj.upper - adj.page_size )

        return True

    def on_stdout(self, fd, cond):
        return self.on_io( fd, cond )

    def on_stderr(self, fd, cond):
        return self.on_io( fd, cond )

    def on_process_exit(self, pid, status):

        status_text=''
        if status:
            dlg = gtk.MessageDialog(self.wiz, 0, gtk.MESSAGE_ERROR, gtk.BUTTONS_OK, _('File transmission failed!') )
            dlg.run()
            dlg.destroy()
            status_text = _('Failed')
        else:
            status_text = _('Finished')

        self.wiz.set_page_title( self.progress_page, status_text )
        self.progress.insert(self.progress.get_end_iter(), '\n%s' % status_text )

        self.wiz.set_page_type( self.progress_page, gtk.ASSISTANT_PAGE_SUMMARY )

    def perform_action(self):
        cmd = ''
        if self.action == ACTION_RECV:
            self.wiz.set_page_title( self.progress_page, _('Receiving...') )
            port = self.xml.get_widget('listen_port').get_text()
            dest_dir = self.xml.get_widget('recv_dir').get_filename()

            self.config.set('history', 'listen_port', port)
            self.config.set('history', 'recv_dir', dest_dir)

            cmd = "nc -l -v --close -p %s | tar -xvz -C '%s'" % (port, dest_dir)
        else:   # ACTION_SEND
            self.wiz.set_page_title( self.progress_page, _('Sending...') )
            host = self.xml.get_widget('dest_host').get_text()
            port = self.xml.get_widget('dest_port').get_text()

            self.config.set('history', 'dest_host', host)
            self.config.set('history', 'dest_port', port)

            files=' '
            dirs=[]
            for it in self.send_list:
                # special char like single quote should be escaped
                files += ("'%s' " % it[1].replace("'", "'\\''") )
                # FIXME: how to escape dir names correctly for transformation pattern?
                dir = os.path.dirname(it[1])
                if not dir in dirs:
                    dirs.append( dir )

            transform='--show-transformed-names '
            # sort the dirs to put the longer ones before shorter ones
            dirs.sort( lambda a, b: len(b)-len(a) )
            # use --transform to strip all prefix of the added files
            for dir in dirs:
                transform += ( "--transform='s,^%s/,,' " % dir )

            # striping dir paths of those files with --transform
            cmd = "tar %s -cvzP %s| nc -w30 -v --close '%s' %s" % (transform, files, host, port)

        self.progress.insert(self.progress.get_end_iter(), '%s %s\n\n%s\n\n' % (_('Execute command:'), _('Processing...'), cmd))
        # print cmd
        (self.pid, self.stdi, self.stdo, self.stde) = gobject.spawn_async( ['sh', '-c', ('exec %s' % cmd)], flags=gobject.SPAWN_SEARCH_PATH|gobject.SPAWN_DO_NOT_REAP_CHILD, standard_input=True, standard_output=True, standard_error=True)
        gobject.child_watch_add(self.pid, self.on_process_exit )
        gobject.io_add_watch( self.stdo, gobject.IO_IN|gobject.IO_HUP|gobject.IO_ERR, self.on_stdout )
        gobject.io_add_watch( self.stde, gobject.IO_IN|gobject.IO_HUP|gobject.IO_ERR, self.on_stderr )

        # enable the Cancel button with very dirty hacks
        def find_cancel_btn(btn, _self):
            if btn and gobject.type_is_a(btn, gtk.Button) and btn.get_label()==gtk.STOCK_CANCEL:
                _self.cancel_btn = btn
        def find_hbox(w, _self):
            if w and gobject.type_is_a(w, gtk.HBox):
                w.forall(find_cancel_btn, _self)
        self.wiz.forall(find_hbox, self)
        if self.cancel_btn:
            self.cancel_btn.set_sensitive(True)

    def init_send(self):
        if not self.send_list:
            self.send_list = gtk.ListStore( gtk.gdk.Pixbuf, gobject.TYPE_STRING )
            self.send_list_view = view = self.xml.get_widget('src_file_list')
            view.set_model( self.send_list )
            self.send_list_view.get_selection().set_mode(gtk.SELECTION_MULTIPLE)

            col = gtk.TreeViewColumn( '', gtk.CellRendererPixbuf(), pixbuf=0 )
            view.append_column( col )

            col = gtk.TreeViewColumn( _('Path'), gtk.CellRendererText(), text=1 )
            view.append_column( col )

            self.file_icon = view.render_icon( gtk.STOCK_FILE, gtk.ICON_SIZE_MENU );
            self.folder_icon = view.render_icon( gtk.STOCK_DIRECTORY, gtk.ICON_SIZE_MENU );

            self.xml.get_widget('add_files').set_image( gtk.image_new_from_stock(gtk.STOCK_FILE, gtk.ICON_SIZE_BUTTON) )
            self.xml.get_widget('add_folders').set_image( gtk.image_new_from_stock(gtk.STOCK_DIRECTORY, gtk.ICON_SIZE_BUTTON) )

    def on_add_send_files(self, btn):
        self.add_send_files( True )

    def on_add_send_folders(self, btn):
        self.add_send_files( False )

    def add_send_files( self, is_file ):
        if is_file:
            action = gtk.FILE_CHOOSER_ACTION_OPEN
            icon = self.file_icon
        else:
            action = gtk.FILE_CHOOSER_ACTION_SELECT_FOLDER
            icon = self.folder_icon

        dlg = gtk.FileChooserDialog(_('Select files to send'), self.wiz, action,(gtk.STOCK_CANCEL, gtk.RESPONSE_CANCEL, gtk.STOCK_OK, gtk.RESPONSE_OK) )
        dlg.set_select_multiple( True )
        if dlg.run() == gtk.RESPONSE_OK:
            sels = dlg.get_filenames()
            for sel in sels:
                self.send_list.append([icon, sel])
        dlg.destroy()
        self.update_send_page()

    def on_remove_send_files(self, btn):
        (list, rows)=self.send_list_view.get_selection().get_selected_rows()
        its=[]
        for row in rows:
            its.append( list.get_iter(row) )
        for it in its:
            list.remove( it )
        self.update_send_page()

    def update_send_page(self):
        # FIXME: more checks are needed!!
        if self.send_list.iter_n_children(None) > 0:
            self.wiz.set_page_complete(self.send_page, True)

    def on_cancel(self, wiz):
        if self.pid > 0:
            # FIXME: This doesn't kill the process of nc and tar... why??
            os.kill(self.pid, signal.SIGKILL)
            os.waitpid(self.pid, 0 )

        self.wiz.destroy()

    def on_close(self, wiz):
        self.wiz.destroy()

    def on_destroy(self, wiz):
        dir=os.path.expanduser('~/.config/gtknetcat')
        if not os.path.exists( dir ):
            os.makedirs(dir)
        cf = open('%s/config' % dir, 'w')
        self.config.write(cf)
        cf.close()

        if self.stde:
            os.close( self.stde )
        if self.stdi:
            os.close( self.stdi )
        gtk.main_quit()

