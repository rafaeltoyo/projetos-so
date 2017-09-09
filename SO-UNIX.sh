# ---------------------------------------------------
# Comando em arquivos no UNIX
# 1
# $USER = rafaeltoyo
# $HOME = /home/$USER

# Listagem sobre o dir /home/$USER
ls $HOME

# Listagem longa (-l)
ls -l $HOME

# Listagem curta com arquivos ocultos (-a)
ls -a $HOME

# Listagem longa de /var/spool/mail ordenado por tamanho (-S)
ls -alS /var/spool/mail

# Listagem longa de /etc ordenada alfabeticamente (padrao)
ls -l /etc

# Listagem longa de /home por datas crescentes (-t desc -> -r reverte)
ls -ltr /home

# Listagem curta de /usr, recursiva (-R) e ordenado pelo tamanho
ls -RS /usr

# 2
mkdir -p a/b/{c/{d/h,e},f/d,j}

# 2.1 Find em todos links simbolicos em /usr
find /usr -type l

# 2.2

# 2.3

# ---------------------------------------------------
# Permissões de acesso em arquivos no UNIX