/*
 *      lxdm.c - main entry of lxdm
 *
 *      Copyright 2009 dgod <dgod.osa@gmail.com>
 *
 *      This program is free software; you can redistribute it and/or modify
 *      it under the terms of the GNU General Public License as published by
 *      the Free Software Foundation; either version 3 of the License, or
 *      (at your option) any later version.
 *
 *      This program is distributed in the hope that it will be useful,
 *      but WITHOUT ANY WARRANTY; without even the implied warranty of
 *      MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *      GNU General Public License for more details.
 *
 *      You should have received a copy of the GNU General Public License
 *      along with this program; if not, write to the Free Software
 *      Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston,
 *      MA 02110-1301, USA.
 */

#define _GNU_SOURCE

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif
#ifndef HAVE_LIBPAM
#define HAVE_LIBPAM 0
#endif
#ifndef HAVE_LIBXMU
#define HAVE_LIBXMU 0
#endif

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <unistd.h>
#include <stdarg.h>
#include <fcntl.h>
#include <pwd.h>
#include <grp.h>
#include <shadow.h>
#include <errno.h>
#include <signal.h>
#include <time.h>
#include <sys/wait.h>
#include <glib.h>
#include <gdk/gdk.h>
#include <gdk/gdkx.h>
#include <X11/Xlib.h>
#include <X11/XKBlib.h>

#include <sys/vt.h>
#include <sys/ioctl.h>

#if HAVE_LIBXMU
#include <X11/Xmu/WinUtil.h>
#endif

#if HAVE_LIBPAM
#include <security/pam_appl.h>
#endif

#if HAVE_LIBCK_CONNECTOR
#include "ck-connector.h"
#endif

#include "lxdm.h"

GKeyFile *config;
static pid_t server;
#if HAVE_LIBCK_CONNECTOR
static CkConnector *ckc;
#endif
static Window *my_xid;
static unsigned int my_xid_n;
static char *self;
static pid_t child;
static int reason;
static char mcookie[33];
static int old_tty=1,tty = 7;

static int get_active_vt(void)
{
    int console_fd;
    struct vt_stat console_state = { 0 };

    console_fd = open("/dev/tty0", O_RDONLY | O_NOCTTY);

    if( console_fd < 0 )
        goto out;

    if( ioctl(console_fd, VT_GETSTATE, &console_state) < 0 )
        goto out;

out:
    if( console_fd >= 0 )
        close(console_fd);

    return console_state.v_active;
}

static void set_active_vt(int vt)
{
    int fd;

    fd = open("/dev/console", O_RDWR);
    if( fd < 0 )
        fd = 0;
    ioctl(fd, VT_ACTIVATE, vt);
    if( fd != 0 )
        close(fd);
}

static gboolean plymouth_is_running(void)
{
	int status;
	gboolean res;

	res=g_spawn_command_line_sync ("/bin/plymouth --ping",NULL,NULL,&status,NULL);
	if(!res) return FALSE;
	return WIFEXITED (status) && WEXITSTATUS (status) == 0;
}

static void plymouth_quit_with_transition(void)
{
	g_spawn_command_line_sync("/bin/plymouth quit --retain-splash",NULL,NULL,NULL,NULL);
}

static void plymouth_quit_without_transition(void)
{
	g_spawn_command_line_sync("/bin/plymouth quit --retain-splash",NULL,NULL,NULL,NULL);
}

static void plymouth_prepare_transition(void)
{
	g_spawn_command_line_sync ("/bin/plymouth deactivate",NULL,NULL,NULL,NULL);
}

void lxdm_get_tty(void)
{
    char *s = g_key_file_get_string(config, "server", "arg", 0);
    int arc;
    char **arg;
    int len;
    int gotvtarg = 0;
    int nr = 0;
    gboolean plymouth;
    
    plymouth=plymouth_is_running();
    if(plymouth) plymouth_prepare_transition();

    old_tty=get_active_vt();
    if( !s ) s = g_strdup("/usr/bin/X");
    g_shell_parse_argv(s, &arc, &arg, 0);
    g_free(s);
    for( len = 0; arg && arg[len]; len++ )
    {
        char *p = arg[len];
        if( !strncmp(p, "vt", 2) && isdigit(p[2]) &&
           ( !p[3] || (isdigit(p[3]) && !p[4]) ) )
        {
            tty = atoi(p + 2);
            gotvtarg = 1;
        }
    }
    if(!gotvtarg)
    {
        /* support plymouth */
        nr = g_file_test("/var/spool/gdm/force-display-on-active-vt", G_FILE_TEST_EXISTS);
        if( nr || g_key_file_get_integer(config, "server", "active_vt", 0) )
            /* use the active vt */
            tty = old_tty;
        if( nr ) unlink("/var/spool/gdm/force-display-on-active-vt");
        if(plymouth)
        {
			nr=1;
			plymouth_quit_with_transition();
		}
    }
    else
    {
		if(plymouth) /* set tty and plymouth running */
			plymouth_quit_without_transition();
	}
    arg = g_renew(char *, arg, len + 10);
    if( !gotvtarg )
        arg[len++] = g_strdup_printf("vt%d", tty);
    arg[len++] = g_strdup("-nolisten");
    arg[len++] = g_strdup("tcp");
    if( nr != 0 )
        arg[len++] = g_strdup("-nr");
    arg[len] = NULL;
    s = g_strjoinv(" ", arg);
    g_strfreev(arg);
    g_key_file_set_string(config, "server", "arg", s);
    g_free(s);
    if(old_tty!=tty)
        set_active_vt(tty);
}

void lxdm_restart_self(void)
{
    reason = 0;
    exit(0);
}

void lxdm_quit_self(void)
{
    reason = 1;
    exit(0);
}

void log_print(char *fmt, ...)
{
    static FILE *log;
    va_list ap;
    if( !fmt )
    {
        if( log )
            fclose(log);
        log = 0;
        return;
    }
    if( !log )
    {
        log = fopen("/var/log/lxdm.log", "w");
        if( !log )
            return;
    }
    va_start(ap, fmt);
    vfprintf(log, fmt, ap);
    va_end(ap);
    fflush(log);
}

GSList *do_scan_xsessions(void)
{
    GSList *xsessions = NULL;
    GDir *d;
    const char *basename;
    GKeyFile *f;

    d = g_dir_open(XSESSIONS_DIR, 0, NULL);
    if( !d )
        return NULL;

    f = g_key_file_new();
    while( ( basename = g_dir_read_name(d) ) != NULL )
    {
        char *file_path;
        gboolean loaded;

        if(!g_str_has_suffix(basename, ".desktop"))
            continue;

        file_path = g_build_filename(XSESSIONS_DIR, basename, NULL);
        loaded = g_key_file_load_from_file(f, file_path, G_KEY_FILE_NONE, NULL);
        g_free(file_path);

        if( loaded )
        {
            char *name = g_key_file_get_locale_string(f, "Desktop Entry", "Name", NULL, NULL);
            if( name )
            {
                char *exec = g_key_file_get_string(f, "Desktop Entry", "Exec", NULL);
                if(exec)
                {
                    Session* sess = g_new( Session, 1 );
                    sess->name = name;
                    sess->exec = exec;
                    sess->desktop_file = g_strdup(basename);
                    if( !strcmp(name, "LXDE") )
                        xsessions = g_slist_prepend(xsessions, sess);
                    else
                        xsessions = g_slist_append(xsessions, sess);
                    continue; /* load next file */
                    g_free(exec);
                }
                g_free(name);
            }
        }
    }
    g_dir_close(d);
    g_key_file_free(f);
    return xsessions;
}

void free_xsessions(GSList *l)
{
    GSList *p;
    Session *sess;

    for( p = l; p; p = p->next )
    {
        sess = p->data;
        g_free(sess->name);
        g_free(sess->exec);
        g_free(sess);
    }
    g_slist_free(l);
}

void create_server_auth(void)
{
    GRand *h;
    const char *digits = "0123456789abcdef";
    int i, r, hex = 0;
    char *authfile;
    char *tmp;

    h = g_rand_new();
    for( i = 0; i < 31; i++ )
    {
        r = g_rand_int(h) % 16;
        mcookie[i] = digits[r];
        if( r > 9 )
            hex++;
    }
    if( (hex % 2) == 0 )
        r = g_rand_int(h) % 10;
    else
        r = g_rand_int(h) % 5 + 10;
    mcookie[31] = digits[r];
    mcookie[32] = 0;
    g_rand_free(h);

    authfile = g_key_file_get_string(config, "base", "authfile", 0);
    if( !authfile )
        authfile = g_strdup("/var/run/lxdm.auth");
    tmp = g_strdup_printf("XAUTHORITY=%s", authfile);
    putenv(tmp);
    g_free(tmp);
    remove(authfile);
    tmp = g_strdup_printf("xauth -q -f %s add %s . %s",
                          authfile, getenv("DISPLAY"), mcookie);
    system(tmp);
    g_free(tmp);
    g_free(authfile);
}

void create_client_auth(char *home)
{
    char *tmp;
    char *authfile;

    if( getuid() == 0 ) /* root don't need it */
        return;

    authfile = g_strdup_printf("%s/.Xauthority", home);
    remove(authfile);
    tmp = g_strdup_printf("xauth -q -f %s add %s . %s",
                          authfile, getenv("DISPLAY"), mcookie);
    system(tmp);
    g_free(authfile);
    g_free(tmp);
}

int lxdm_auth_user(char *user, char *pass, struct passwd **ppw)
{
    struct passwd *pw;
    struct spwd *sp;
    char *real;
    char *enc;
    if( !user )
        return AUTH_ERROR;
    if( !user[0] )
        return AUTH_BAD_USER;
    pw = getpwnam(user);
    endpwent();
    if( !pw )
        return AUTH_BAD_USER;
    if( !pass )
    {
        *ppw = pw;
        return AUTH_SUCCESS;
    }
    sp = getspnam(user);
    if( !sp )
        return AUTH_FAIL;
    endspent();
    real = sp->sp_pwdp;
    if( !real || !real[0] )
    {
        if( !pass[0] )
        {
            *ppw = pw;
            return AUTH_SUCCESS;
        }
        else
            return AUTH_FAIL;
    }
    enc = crypt(pass, real);
    if( strcmp(real, enc) )
        return AUTH_FAIL;
    if( strstr(pw->pw_shell, "nologin") )
        return AUTH_PRIV;
    *ppw = pw;
    return AUTH_SUCCESS;
}

#if HAVE_LIBPAM
static pam_handle_t *pamh;
static struct pam_conv conv;

void setup_pam_session(struct passwd *pw,char *session_name)
{
    int err;
    char x[256];
   
    if( PAM_SUCCESS != pam_start("lxdm", pw->pw_name, &conv, &pamh) )
    {
        pamh = NULL;
        return;
    }
    sprintf(x, "tty%d", tty);
    pam_set_item(pamh, PAM_TTY, x);
#ifdef PAM_XDISPLAY
    pam_set_item( pamh, PAM_XDISPLAY, getenv("DISPLAY") );
#endif

	if(session_name && session_name[0])
	{
		char *env;
		env = g_strdup_printf ("DESKTOP_SESSION=%s", session_name);
		pam_putenv (pamh, env);
		g_free (env);
	}
    err = pam_open_session(pamh, 0); /* FIXME pam session failed */
    if( err != PAM_SUCCESS )
        log_print( "pam open session error \"%s\"\n", pam_strerror(pamh, err) );
}

void close_pam_session(void)
{
    int err;
    if( !pamh ) return;
    err = pam_close_session(pamh, 0);
    pam_end(pamh, err);
    pamh = NULL;
}

#if 0
void append_pam_environ(char **env)
{
	int i,j,n;
	char **penv;
	if(!pamh) return;
	penv=pam_getenvlist(pamh);
	if(!penv) return;
	for(i=0;penv[i]!=NULL;i++)
	{
		n=strcspn(penv[i],"=")+1;
		for(j=0;env[j]!=NULL;j++)
		{
			if(!strncmp(penv[i],env[j],n))
				break;
			if(env[j+1]==NULL)
			{
				env[j+1]=g_strdup(penv[i]);
				env[j+2]=NULL;
				break;
			}
		}
		free(penv[i]);
	}
	free(penv);
}
#endif

#endif

void switch_user(struct passwd *pw, char *run, char **env)
{
    if( !pw || initgroups(pw->pw_name, pw->pw_gid) ||
        setgid(pw->pw_gid) || setuid(pw->pw_uid) || setsid() == -1 )
        exit(EXIT_FAILURE);
    chdir(pw->pw_dir);
    create_client_auth(pw->pw_dir);
    execle("/etc/lxdm/Xsession", "/etc/lxdm/Xsession", run, NULL, env);
    exit(EXIT_FAILURE);
}

void get_lock(void)
{
    FILE *fp;
    char *lockfile;

    lockfile = g_key_file_get_string(config, "base", "lock", 0);
    if( !lockfile ) lockfile = g_strdup("/var/run/lxdm.pid");

    fp = fopen(lockfile, "r");
    if( fp )
    {
        int pid;
        int ret;
        ret = fscanf(fp, "%d", &pid);
        fclose(fp);
        if( ret == 1 )
            if( kill(pid, 0) == 0 || (ret == -1 && errno == EPERM) )
                exit(EXIT_SUCCESS);
    }
    fp = fopen(lockfile, "w");
    if( !fp )
        exit(EXIT_FAILURE);
    fprintf( fp, "%d", getpid() );
    fclose(fp);
    g_free(lockfile);
}

void put_lock(void)
{
    FILE *fp;
    char *lockfile;

    lockfile = g_key_file_get_string(config, "base", "lock", 0);
    if( !lockfile ) lockfile = g_strdup("/var/run/lxdm.pid");
    fp = fopen(lockfile, "r");
    if( fp )
    {
        int pid;
        int ret;
        ret = fscanf(fp, "%d", &pid);
        fclose(fp);
        if( ret == 1 && pid == getpid() )
            remove(lockfile);
    }
    g_free(lockfile);
}

void stop_pid(int pid)
{
    if( pid <= 0 ) return;
    if( killpg(pid, SIGTERM) < 0 )
        killpg(pid, SIGKILL);
    if( kill(pid, 0) == 0 )
    {
        if( kill(pid, SIGTERM) )
            kill(pid, SIGKILL);
        while( 1 )
        {
            int wpid, status;
            wpid = wait(&status);
            if( pid == wpid ) break;
        }
    }
    while( waitpid(-1, 0, WNOHANG) > 0 ) ;
}

static void on_xserver_stop(GPid pid, gint status, gpointer data)
{
    stop_pid(server);
    server = -1;
    lxdm_restart_self();
}

static void set_numlock(void)
{
	Display *dpy;
	XkbDescPtr xkb;
	unsigned int mask;
	int on;
	int i;
	if(!g_key_file_has_key(config,"base","numlock",NULL))
		return;
	on=g_key_file_get_integer(config,"base","numlock",0);
	dpy=gdk_x11_get_default_xdisplay();
	if(!dpy) return;
	xkb = XkbGetKeyboard( dpy, XkbAllComponentsMask, XkbUseCoreKbd );
	if(!xkb) return;
	if(!xkb->names)
	{
		XkbFreeKeyboard(xkb,0,True);
		return;
	}
	for(i = 0; i < XkbNumVirtualMods; i++)
	{
		char *s=XGetAtomName( xkb->dpy, xkb->names->vmods[i]);
		if(!s) continue;
		if(strcmp(s,"NumLock")) continue;
		XkbVirtualModsToReal( xkb, 1 << i, &mask );
		break;
	}
	XkbFreeKeyboard( xkb, 0, True );
	XkbLockModifiers ( dpy, XkbUseCoreKbd, mask, (on?mask:0));
}

void startx(void)
{
    char *arg;
    char **args;

    if( !getenv("DISPLAY") )
        putenv("DISPLAY=:0");

    create_server_auth();

    arg = g_key_file_get_string(config, "server", "arg", 0);
    if( !arg ) arg = g_strdup("/usr/bin/X");
    args = g_strsplit(arg, " ", -1);
    g_free(arg);

    server = vfork();

    switch( server )
    {
    case 0:
        setpgid( 0, getpid() );
        execvp(args[0], args);
        break;
    case -1:
        exit(EXIT_FAILURE);
        break;
    default:
        break;
    }
    g_strfreev(args);
    g_child_watch_add(server, on_xserver_stop, 0);
}

void exit_cb(void)
{
    if( child > 0 )
    {
        killpg(child, SIGHUP);
        stop_pid(child);
        child = -1;
    }
#if HAVE_LIBPAM
    close_pam_session();
#endif
    if( server > 0 )
    {
        stop_pid(server);
        server = -1;
    }
    put_lock();
    if( reason == 0 )
        execlp(self, self, NULL);
    set_active_vt(old_tty);
}

int CatchErrors(Display *dpy, XErrorEvent *ev)
{
    return 0;
}

void get_my_xid(void)
{
    Window dummy, parent;
    Display *Dpy = gdk_x11_get_default_xdisplay();
    Window Root = gdk_x11_get_default_root_xwindow();
    XQueryTree(Dpy, Root, &dummy, &parent, &my_xid, &my_xid_n);
}

int is_my_id(XID id)
{
    int i;
    if( !my_xid )
        return 0;
    for( i = 0; i < my_xid_n; i++ )
        if( id == my_xid[i] ) return 1;
    return 0;
}

void free_my_xid(void)
{
    XFree(my_xid);
    my_xid = 0;
}

void stop_clients(int top)
{
    Window dummy, parent;
    Window *children;
    unsigned int nchildren;
    unsigned int i;
    XWindowAttributes attr;
    Display *Dpy = gdk_x11_get_default_xdisplay();
    Window Root = gdk_x11_get_default_root_xwindow();

    XSync(Dpy, 0);
    XSetErrorHandler(CatchErrors);

    nchildren = 0;
    XQueryTree(Dpy, Root, &dummy, &parent, &children, &nchildren);
    if( !top )
    {
        for( i = 0; i < nchildren; i++ )
        {
            if( XGetWindowAttributes(Dpy, children[i], &attr) && (attr.map_state == IsViewable) )
#if HAVE_LIBXMU
                children[i] = XmuClientWindow(Dpy, children[i]);
#else
                children[i] = children[i];
#endif
            else
                children[i] = 0;
        }
    }

    for( i = 0; i < nchildren; i++ )
        if( children[i] && !is_my_id(children[i]) )
            XKillClient(Dpy, children[i]);
    //printf("kill %d\n",i);
    XFree( (char *)children );
    XSync(Dpy, 0);
    XSetErrorHandler(NULL);
}

static void on_session_stop(GPid pid, gint status, gpointer data)
{
    int code = WEXITSTATUS(status);

    killpg(pid, SIGHUP);
    stop_pid(pid);
    child = -1;

    if( server > 0 )
    {
        /* FIXME just work around lxde bug of focus can't set */
        //stop_clients(0);
        stop_clients(1);
        free_my_xid();
    }
#if HAVE_LIBPAM
    close_pam_session();
#endif
#if HAVE_LIBCK_CONNECTOR
    if( ckc != NULL )
    {
        DBusError error;
        dbus_error_init(&error);
        ck_connector_close_session(ckc, &error);
        unsetenv("XDG_SESSION_COOKIE");
    }
#endif
    if( code == 0 )
        /* xterm will quit use this, but we shul not quit here */
        /* so wait someone to kill me may better */
        //lxdm_quit_self();
        sleep(2);

    ui_prepare();
}

static void replace_env(char** env, const char* name, const char* new_val)
{
    register char** penv;
    for(penv = env; *penv; ++penv)
    {
        if(g_str_has_prefix(*penv, name))
        {
            g_free(*penv);
            *penv = g_strconcat(name, new_val, NULL);
            return;
        }
    }
    *penv = g_strconcat(name, new_val, NULL);
    *(penv + 1) = NULL;
}

gboolean lxdm_get_session_info(char *session,char **pname,char **pexec)
{
	char *name=NULL,*exec=NULL;
	if(!session || !session[0])
	{
		name=g_key_file_get_string(config, "base", "session", 0);
		if(!name && getenv("PREFERRED"))
			name = g_strdup(getenv("PREFERRED"));
		if(!session && getenv("DESKTOP"))
			name = g_strdup(getenv("DESKTOP"));
		if(!name) name=g_strdup("LXDE");
	}
	else
	{
		char *p=strrchr(session,'.');
		if(p && !strcmp(p,".desktop"))
		{
			GKeyFile *cfg=g_key_file_new();
			if(!g_key_file_load_from_file(cfg,session,G_KEY_FILE_NONE,NULL))
			{
				g_key_file_free(cfg);
				return FALSE;
			}
			name=g_key_file_get_string(cfg,"Desktop Entry","Name",NULL);
			exec=g_key_file_get_string(cfg,"Desktop Entry","Exec",NULL);
			g_key_file_free(cfg);
			if(!name || !exec)
			{
				g_free(name);
				g_free(exec);
				return FALSE;
			}			
		}
		else
		{
			name=g_strdup(session);
		}
	}
	if(name && !exec)
	{
		if(!strcmp(name,"LXDE"))
			exec = g_strdup("startlxde");
		else if( !strcmp(name, "GNOME") )
			exec = g_strdup("gnome-session");
		else if( !strcmp(name, "KDE") )
			exec = g_strdup("startkde");
		else if( !strcmp(name, "XFCE") )
			exec = g_strdup("startxfce4");
		else
			exec=g_strdup(name);
	}
	if(pname) *pname=name;
	if(pexec) *pexec=exec;
	return TRUE;
}

void lxdm_do_login(struct passwd *pw, char *session, char *lang)
{
	char *session_name=0,*session_exec=0;
    int pid;
    
    if(!lxdm_get_session_info(session,&session_name,&session_exec))
    {
		ui_prepare();
    	return;
	}

    if( pw->pw_shell[0] == '\0' )
    {
        setusershell();
        strcpy( pw->pw_shell, getusershell() );
        endusershell();
    }
#if HAVE_LIBPAM
    setup_pam_session(pw,session_name);
#endif
#if HAVE_LIBCK_CONNECTOR
    if( ckc != NULL )
    {
        DBusError error;
        char x[256], *d, *n;
        sprintf(x, "/dev/tty%d", tty);
        dbus_error_init(&error);
        d = x; n = getenv("DISPLAY");
        if( ck_connector_open_session_with_parameters(ckc, &error,
                                                      "unix-user", &pw->pw_uid,
                                                      "display-device", &d,
                                                      "x11-display-device", &d,
                                                      "x11-display", &n,
                                                      NULL) )
            setenv("XDG_SESSION_COOKIE", ck_connector_get_cookie(ckc), 1);
    }
#endif
    get_my_xid();
    child = pid = fork();
    if( child == 0 )
    {
        char** env, *path;
        int n_env = g_strv_length(environ), i;
        /* copy all environment variables and override some of them */
        env = g_new(char*, n_env + 1 + 13);
        for( i = 0; i < n_env; ++i )
            env[i] = g_strdup(environ[i]);
        env[i] = NULL;

        replace_env(env, "HOME=", pw->pw_dir);
        replace_env(env, "SHELL=", pw->pw_shell);
        replace_env(env, "USER=", pw->pw_name);
        replace_env(env, "LOGNAME=", pw->pw_name);

        /* override $PATH if needed */
        path = g_key_file_get_string(config, "base", "path", 0);
        if( G_UNLIKELY(path) && path[0] )
        	replace_env(env, "PATH=", path);
	else
		replace_env(env, "PATH=","/usr/local/bin:/bin:/usr/bin");
        g_free(path);
        /* optionally override $LANG, $LC_MESSAGES, and $LANGUAGE */
        if( lang && lang[0] )
        {
            replace_env(env, "LANG=", lang);
            replace_env(env, "LC_MESSAGES=", lang);
            replace_env(env, "LANGUAGE=", lang);
        }
        
#if HAVE_LIBPAM
#if 0
		append_pam_environ(env);
#endif
#endif

#if 0
        if( !session || !session[0] ) /* this means use default session */
            session = g_key_file_get_string(config, "base", "session", 0);
        if( !session && getenv("PREFERRED") )
            session = g_strdup( getenv("PREFERRED") );
        if( !session && getenv("DESKTOP") )
        {
            char *p = getenv("DESKTOP");
            if( !strcmp(p, "LXDE") )
                session = g_find_program_in_path("startlxde");
            else if( !strcmp(p, "GNOME") )
                session = g_find_program_in_path("gnome-session");
            else if( !strcmp(p, "KDE") )
                session = g_find_program_in_path("startkde");
            else if( !strcmp(p, "XFCE") )
                session = g_strdup("startxfce4");
            else
                session = g_strdup(p);
        }
        if( !session )
            session = g_strdup("");

        switch_user(pw, session, env);
#else
		switch_user(pw, session_exec, env);
#endif
        reason = 4;
        exit(EXIT_FAILURE);
    }
    g_free(session_name);
    g_free(session_exec);
    g_child_watch_add(pid, on_session_stop, 0);
}

void lxdm_do_reboot(void)
{
    char *cmd;
    cmd = g_key_file_get_string(config, "cmd", "reboot", 0);
    if( !cmd ) cmd = g_strdup("reboot");
    reason = 1;
    system(cmd);
    g_free(cmd);
    lxdm_quit_self();
}

void lxdm_do_shutdown(void)
{
    char *cmd;
    cmd = g_key_file_get_string(config, "cmd", "shutdown", 0);
    if( !cmd ) cmd = g_strdup("shutdown -h now");
    reason = 1;
    system(cmd);
    g_free(cmd);
    lxdm_quit_self();
}

int lxdm_cur_session(void)
{
    return child;
}

int lxdm_do_auto_login(void)
{
    struct passwd *pw;
    char *user;

    user = g_key_file_get_string(config, "base", "autologin", 0);
    if( !user )
        return 0;
    if( AUTH_SUCCESS != lxdm_auth_user(user, 0, &pw) )
        return 0;
    lxdm_do_login(pw, NULL, NULL);
    return 1;
}

void sig_handler(int sig)
{
    log_print("catch signal %d\n", sig);
    switch( sig )
    {
    case SIGTERM:
    case SIGINT:
        lxdm_quit_self();
        break;
    default:
        break;
    }
}

void set_signal(void)
{
    signal(SIGQUIT, sig_handler);
    signal(SIGTERM, sig_handler);
    signal(SIGKILL, sig_handler);
    signal(SIGINT, sig_handler);
    signal(SIGHUP, sig_handler);
    signal(SIGPIPE, sig_handler);
    signal(SIGUSR1, sig_handler);
    signal(SIGALRM, sig_handler);
}

#if HAVE_LIBCK_CONNECTOR
void init_ck(void)
{
    ckc = ck_connector_new();
}
#endif

int main(int arc, char *arg[])
{
    int tmp;
    int daemonmode = 0;

    if( getuid() != 0 )
    {
        printf("only root is allowed to use this program\n");
        exit(EXIT_FAILURE);
    }

    while( ( tmp = getopt(arc, arg, "hd") ) != EOF )
    {
        switch( tmp )
        {
        case 'd':
            daemonmode = 1;
            break;
        case 'h':
            printf("usage:  lxdm [options ...]\n");
            printf("options:\n");
            printf("    -d: daemon mode\n");
            exit(EXIT_SUCCESS);
            break;
        }
    }

    if( daemonmode )
        daemon(1, 1);

    self = arg[0];

    config = g_key_file_new();
    g_key_file_load_from_file(config, CONFIG_FILE, G_KEY_FILE_NONE, NULL);

    get_lock();
    atexit(exit_cb);

    set_signal();
    lxdm_get_tty();
    startx();

    for( tmp = 0; tmp < 200; tmp++ )
    {
        if( gdk_init_check(0, 0) )
            break;
        g_usleep(50 * 1000);
    }
    if( tmp >= 200 )
        exit(EXIT_FAILURE);
    set_numlock();

#if HAVE_LIBCK_CONNECTOR
    init_ck();
#endif

    lxdm_do_auto_login();

    ui_main();

    lxdm_restart_self();

    return 0;
}
