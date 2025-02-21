#!/bin/sh

# This script is called from systemd unit file to mount or unmount
# a USB drive.

usage()
{
    echo "Usage: $0 {add|remove} device_name (e.g. sdb1)"
    exit 1
}

if [[ $# -ne 2 ]]; then
    usage
fi

ACTION=$1
DEVBASE=$2
DEVICE="/dev/${DEVBASE}"
MOUNT_PREFIX="/mnt/media"
SYMLINK_PREFIX="/data"
SYMLINK="${SYMLINK_PREFIX}/${DEVBASE}"
ADD_LINK="false"

# See if this drive is already mounted, and if so where
MOUNT_POINT=$(mount | grep ${DEVICE} | awk '{ print $3 }')

do_mount()
{
    if [[ -n ${MOUNT_POINT} ]]; then
        echo "Warning: ${DEVICE} is already mounted at ${MOUNT_POINT}"
        exit 1
    fi

    # Get info for this drive: $ID_FS_UUID and $ID_FS_TYPE
    eval $(blkid -o udev ${DEVICE} | cut -d ':' -f 2 | grep -iw -e "ID_FS_UUID" -e "ID_FS_TYPE")

    # Figure out a mount point to use
    LABEL=${ID_FS_UUID}
    if [[ -z "${LABEL}" ]]; then
        LABEL=${DEVBASE}
        ADD_LINK="false"
    elif grep -q " ${MOUNT_PREFIX}/${LABEL} " /etc/mtab; then
        # Already in use, make a unique one
        LABEL+="-${DEVBASE}"
    fi
    MOUNT_POINT="${MOUNT_PREFIX}/${LABEL}"

    echo "Mount point: ${MOUNT_POINT}"

    mkdir -p ${MOUNT_POINT}

    # Global mount options
    OPTS="rw,relatime"

    # File system type specific mount options
    if [[ ${ID_FS_TYPE} == "vfat" ]]; then
        OPTS+=",users,gid=100,umask=000,shortname=mixed,utf8=1,flush"
    fi

    if ! mount -o ${OPTS} ${DEVICE} ${MOUNT_POINT}; then
        echo "Error mounting ${DEVICE} (status = $?)"
        rmdir ${MOUNT_POINT}
        exit 1
    fi

    # Add symbolic link
    if [[ ${ADD_LINK} == "true" ]]; then
        if [[ -L ${SYMLINK} ]]; then
            rm ${SYMLINK}
        fi
        ln -s ${MOUNT_POINT} ${SYMLINK}
    fi

    echo "**** Mounted ${DEVICE} at ${MOUNT_POINT} ****"
}

do_unmount()
{
    if [[ -z ${MOUNT_POINT} ]]; then
        echo "Warning: ${DEVICE} is not mounted"
    else
        #umount -l ${DEVICE}
        umount ${DEVICE}
        echo "**** Unmounted ${DEVICE} from ${MOUNT_POINT}"
        rmdir ${MOUNT_POINT}
        # remove symbolic link
        if [[ ${ADD_LINK} == "true" ]]; then
            if [[ -L ${SYMLINK} ]]; then
                rm ${SYMLINK}
            fi
        fi
    fi
}

case "${ACTION}" in
    add)
        do_mount
        ;;
    remove)
        do_unmount
        ;;
    *)
        usage
        ;;
esac

exit 0