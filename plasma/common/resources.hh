#ifndef RESOURCES_HPP
# define RESOURCES_HPP

# define INFINIT_HOST_PROTOCOL "http"
# define INFINIT_HOST "hotgloupi.fr"
# define INFINIT_DOWNLOAD_DIR "/dl/"

# define INFINIT_UPDATER_FILENAME "updater.exe"
# define INFINIT_UPDATER_URI (INFINIT_HOST_PROTOCOL "://" INFINIT_HOST INFINIT_DOWNLOAD_DIR INFINIT_UPDATER_FILENAME)

# define INFINIT_RELEASE_URI (INFINIT_HOST_PROTOCOL "://" INFINIT_HOST INFINIT_DOWNLOAD_DIR "last_release.xml")

# ifdef WIN32
#  define INFINIT_HOME "Infinit"
# else
#  define INFINIT_HOME ".infinit"
# endif

#endif // RESOURCES_HPP
