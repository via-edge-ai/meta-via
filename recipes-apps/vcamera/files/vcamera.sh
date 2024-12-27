#!/bin/bash

VERSION="0.0.4"
DEBUG=0

#input parameter
CAMTYPE="csi"
CAMID="0"
VCID="0"
MODE="preview"
WIDTH="1280"
HEIGHT="720"
FPS="false"
ENCTYPE="h264"
ENCSTRING="v4l2h264enc ! h264parse"

#CSI camera device
CSI_NAME=("mtkcam" "mtkcam" "mtkcam")
CSI_TYPE=("mtkcam" "mtkcam" "mtkcam")   #mtkcam or bridge
BR_COUNT=0
MCAM_COUNT=0
FILE_NAME=""

#USB camera device
UVC_NAME=""
UVC_DEV=""

#Gstreamer image format
H264_ENCODE="v4l2h264enc ! h264parse"
H265_ENCODE="v4l2h265enc ! h265parse"

SEN_W="1280"
SEN_H="720"

export MEDIA_DEV=$(v4l2-ctl -z platform:$(basename `find /sys/bus/platform/devices/ -name "*camisp"`) --list-devices | grep media)

vcamera_help()
{
	echo "\

	[] need| optional
	{} deploy

	usage:
		${0} {[OPTIONS {VALUE}] ...}
		
	(e.g.)
		${0} -t csi -c 0
		${0} -t csi -c 1 -r 1920x1080 -m encode -e h264
		${0} -t usb -c 0 -r 1920x1080 -m encode -e h265 

	Description:options:
	
		--help
			Display this message

		--list
			List all camera include csi and usb
			
		-t
			Set camera type
			(Value: csi or usb)
			(Default: csi)

		-c
			Set csi port num and VC id
			VC id is used when connect bridge
			(csi port Value: 0~2)
			(vc id Value: 0~3)
			(Default: 0:0)

		-r
			Set display resolution
			(Value: WidthxHeight)
			(Default: 1280x720)

		-m
			Set camera mode
			(Value: preview capture encode)
			(Default: preview)"

    echo "----------------------------------------"
}

get_board_name()
{
	local name="unknown"
	if [ -e "/etc/hostname" ]; then
		name=`cat /etc/hostname | tr -d ' \t\n\r'`
	fi
	printf "${name}"
}

find_csi_port_dev()
{
	#find the camera module on seninf(csi) port
	#seninf-0
	CSI_NAME[0]=$(media-ctl -d ${MEDIA_DEV} -p | grep -E "entity[ \t]+[0-9]+:[ \t]seninf-0" -A 5 | \
		grep "<-" | awk {'print $2'} | tr -d '"' | tr ":" " "| awk {'print $1'})
	#seninf-1
	CSI_NAME[1]=$(media-ctl -d ${MEDIA_DEV} -p | grep -E "entity[ \t]+[0-9]+:[ \t]seninf-1" -A 5 | \
		grep "<-" | awk {'print $2'} | tr -d '"' | tr ":" " "| awk {'print $1'})
	#seninf-2
	CSI_NAME[2]=$(media-ctl -d ${MEDIA_DEV} -p | grep -E "entity[ \t]+[0-9]+:[ \t]seninf-2" -A 5 | \
		grep "<-" | awk {'print $2'} | tr -d '"' | tr ":" " "| awk {'print $1'})

	local index=0
	while [ $index -lt 3 ]
	do
		if [ "${CSI_NAME[$index]}" = "" ]; then
			#echo "seninf-$index is no devices"
			CSI_NAME[$index]="DISABLE"
			CSI_TYPE[$index]="disable"
		elif [[ "${CSI_NAME[$index]}" == "sensor"* ]]; then
			#echo "seninf-$index is mtkcam"
			#local tmp=$(echo "${CSI_NAME[$index]}" | tr -d [a-z])
			#CSI_NAME[$index]=$(echo "mtkcam$tmp")
			CSI_TYPE[$index]="mtkcam"
			(( MCAM_COUNT++ ))
		else
			#echo "seninf-$index is bridge(${CSI_NAME[$index]})"
			CSI_TYPE[$index]="bridge"
			(( BR_COUNT++ ))
		fi
		(( index++ ))
	done

	#sort the sensor id to map mtkcam id
	index=0
	local tmp_idx=0
	local tmp=("" "" "")
	#find the mtkcam and copy to tmp array
	while [ $index -lt 3 ]
	do
		if [ "${CSI_TYPE[$index]}" = "mtkcam" ]; then
			tmp[$tmp_idx]=${CSI_NAME[$index]}
			(( tmp_idx++ ))
		fi
		(( index++ ))
	done
	#sort the tmp array
	IFS=$'\n'
	local sorted=($(sort <<<"${tmp[*]}"))
	unset IFS

	#map sensor id to mtkcam id
	index=0
	tmp_idx=0
	while [ $tmp_idx -lt ${#sorted[@]} ]
	do
		while [ $index -lt 3 ]
		do
			if [ "${sorted[$tmp_idx]}" == "${CSI_NAME[$index]}" ]; then
				#change the csi name from sensor to mtkcam
				CSI_NAME[$index]=$(echo "mtkcam$tmp_idx")
			fi
			(( index++ ))
		done
		(( tmp_idx++ ))
	done
	if [ $DEBUG -eq 1 ]; then
		echo "${CSI_NAME[@]}"
		echo "${CSI_TYPE[@]}"
		echo "MTKCAM count: $MCAM_COUNT"
		echo "BRIDGE count: $BR_COUNT"
	fi

}

list_csi_camera()
{
	local idx=0
	local platform='get_broad_name'
	echo "==========CSI camera list=========="
	while [ $idx -lt 3 ]
	do
		if [ "${CSI_TYPE[$idx]}" == "mtkcam" ]; then
			local videodev=(`v4l2-ctl --list-devices | grep -P "mtk-v4l2-camera.*${CSI_NAME[$idx]}" -A 3 | grep -P "video" | tr -d "\n"`)
			echo -e "CSI port $idx => ${CSI_NAME[$idx]}"
			echo -e "\tpreview node: ${videodev[0]}"
			echo -e "\tcapture node: ${videodev[2]}"
			echo -e "\tencode  node: ${videodev[1]}"
		elif [ "${CSI_TYPE[$idx]}" == "bridge" ]; then
			echo -e "CSI port $idx => ${CSI_NAME[$idx]}"
			videodev=(`for i in {0..5}; do media-ctl -d ${MEDIA_DEV} --entity "mtk-cam camsv-$i main-stream"; done | tr "\n" " "`)
			if [ $BR_COUNT -eq 1 ] || [ $idx -eq 1 ]; then
				echo -e "\tVC0 node: ${videodev[0]}"
				#RN6752 only use one camera
				if [ ${CSI_NAME[$idx]} == "rn6752.5-002c" ]; then
					(( idx++ ))
					continue
				fi
				echo -e "\tVC1 node: ${videodev[1]}"
				echo -e "\tVC2 node: ${videodev[2]}"
				echo -e "\tVC3 node: ${videodev[3]}"
			elif [ $BR_COUNT -eq 2 ]; then
				echo -e "\tVC0 node: ${videodev[4]}"
				#RN6752 only use one camera
				if [ ${CSI_NAME[$idx]} == "rn6752.5-002c" ]; then
					(( idx++ ))
					continue
				fi
				echo -e "\tVC1 node: ${videodev[5]}"
				if [ $platform == "som-5000" ]; then #som-5000 support 8 camsv
					echo -e "\tVC2 node: ${videodev[6]}"
					echo -e "\tVC3 node: ${videodev[7]}"
				fi
			fi
		else
			echo -e "CSI port $idx => disable"
		fi
		(( idx++ ))
	done
	echo "==================================="
}

list_usb_camera()
{
	UVC_NAME=(`for i in {0..5}; do v4l2-ctl --list-devices | grep -P "usb-*.$i" | tr  " " "-"; done`)
	UVC_DEV=(`for i in {0..5}; do v4l2-ctl --list-devices | grep -P "usb-*.$i" -A 1 | grep -P "video"; done`)
	if [ "${UVC_NAME[0]}" == "" ]; then
		echo "Can not find USB Camera"
		return
	fi

	echo -e "\tUSB CAMERA LIST:"
	local ID=0
	while [ "${UVC_NAME[$ID]}" != "" ]
	do
		echo -e "\t\tCAM$ID: ${UVC_NAME[$ID]}"
		if [ $1 -eq 1 ]; then
			v4l2-ctl -d ${UVC_DEV[$ID]} --list-formats-ext
		fi
		(( ID++ ))
	done
}

get_resolution()
{
	local DEV_NODE=$1
	local FPS_REQ=$2
	local FMT_S=$3
	local LINE=$4

	if [ -f test_file ]; then
		rm -f test_file
	fi

	if [ -f file_done ]; then
		rm -f file_done
        fi

	TMP=$(v4l2-ctl -d $DEV_NODE --list-formats-ext | grep -P "$FMT_S" -A $LINE | grep -P "Size" | awk {'print $3'})
	echo "${TMP[0]}" >> "test_file"
	sort -nr test_file -o file_done
	declare -a RESOLUTION=(`for i in {0..0}; do cat file_done| awk {'print $0'}; done`)	

	#echo "====================================="
	ID=0
	while [ "${RESOLUTION[$ID]}" != "" ] && [ "${FPS_REQ}" != "0" ]
	do
		FPS30=$(v4l2-ctl -d $DEV_NODE --list-formats-ext | grep -P "${RESOLUTION[$ID]}" -A 5 | grep "$FPS_REQ")
		if [ "$FPS30" != "" ]; then
			#echo "$FPS30"
			break
		fi
		(( ID++ ))
	done
	#echo "====================================="

	SEN_W=$(echo ${RESOLUTION[$ID]} | tr "x" " " |  awk {'print $1'})
	SEN_H=$(echo ${RESOLUTION[$ID]} | tr "x" " " |  awk {'print $2'})

	rm -f test_file
	rm -f file_done
}

setup_camsv_media_links()
{
	local media_dev="${MEDIA_DEV}"
	local board_name=`get_board_name`
	local idx=0

	#Set all Multi-Channel device media link pipeline
	#Maybe can do this when boot on
	if [ $BR_COUNT -eq 1 ]; then
		#find the bridge on which csi port
		idx=0
		while [ $idx -lt 3 ]
		do
			if [ "${CSI_TYPE[$idx]}" == "bridge" ]; then
				echo "Set ${CSI_NAME[$idx]} media link to seninf-$idx"
				if [ "${CSI_NAME[$idx]}" == "rn6752.5-002c" ]; then
					media-ctl -d "${media_dev}" -l "'${CSI_NAME[$idx]}':0 -> 'seninf-$idx':0 [1]"
				else
					media-ctl -d "${media_dev}" -l "'${CSI_NAME[$idx]}':4 -> 'seninf-$idx':0 [1]"
					media-ctl -d "${media_dev}" -l "'seninf-$idx':2 -> 'mtk-cam camsv-1':0 [5]"
					media-ctl -d "${media_dev}" -l "'seninf-$idx':3 -> 'mtk-cam camsv-2':0 [5]"
					media-ctl -d "${media_dev}" -l "'seninf-$idx':4 -> 'mtk-cam camsv-3':0 [5]"
				fi
				media-ctl -d "${media_dev}" -l "'seninf-$idx':1 -> 'mtk-cam camsv-0':0 [5]"

				media-ctl -d "${media_dev}" -V "'seninf-$idx':0 [fmt:UYVY8_1X16/1920x1080 field:none colorspace:srgb]"
				media-ctl -d "${media_dev}" -V "'seninf-$idx':1 [fmt:YUYV8_1X16/1920x1080 field:none colorspace:srgb]"
				media-ctl -d "${media_dev}" -V "'seninf-$idx':2 [fmt:YUYV8_1X16/1920x1080 field:none colorspace:srgb]"
				media-ctl -d "${media_dev}" -V "'seninf-$idx':3 [fmt:YUYV8_1X16/1920x1080 field:none colorspace:srgb]"
				media-ctl -d "${media_dev}" -V "'seninf-$idx':4 [fmt:YUYV8_1X16/1920x1080 field:none colorspace:srgb]"
				media-ctl -d "${media_dev}" -V "'mtk-cam camsv-0':0 [fmt:YUYV8_1X16/1920x1080 field:none colorspace:srgb]"
				media-ctl -d "${media_dev}" -V "'mtk-cam camsv-1':0 [fmt:YUYV8_1X16/1920x1080 field:none colorspace:srgb]"
				media-ctl -d "${media_dev}" -V "'mtk-cam camsv-2':0 [fmt:YUYV8_1X16/1920x1080 field:none colorspace:srgb]"
				media-ctl -d "${media_dev}" -V "'mtk-cam camsv-3':0 [fmt:YUYV8_1X16/1920x1080 field:none colorspace:srgb]"
				media-ctl -d "${media_dev}" -V "'mtk-cam camsv-0':1 [fmt:YUYV8_1X16/1920x1080 field:none colorspace:srgb]"
				media-ctl -d "${media_dev}" -V "'mtk-cam camsv-1':1 [fmt:YUYV8_1X16/1920x1080 field:none colorspace:srgb]"
				media-ctl -d "${media_dev}" -V "'mtk-cam camsv-2':1 [fmt:YUYV8_1X16/1920x1080 field:none colorspace:srgb]"
				media-ctl -d "${media_dev}" -V "'mtk-cam camsv-3':1 [fmt:YUYV8_1X16/1920x1080 field:none colorspace:srgb]"
			fi
			(( idx++ ))
		done
	else
		#for som-7000, Hardcore setting like below
		#csi port 1 vc0~3 -> camsv0~3
		#csi port 0 vc0~1 -> camsv4~5
		idx=1 #first set csi port 1 media link
		while [ $idx -ge 0 ]
		do
			#echo "Set ${CSI_NAME[$idx]} media link to seninf-$idx"
			media-ctl -d "${media_dev}" -l "'${CSI_NAME[$idx]}':4 -> 'seninf-$idx':0 [1]"
			media-ctl -d "${media_dev}" -V "'seninf-$idx':0 [fmt:UYVY8_1X16/1920x1080 field:none colorspace:srgb]"
			media-ctl -d "${media_dev}" -V "'seninf-$idx':1 [fmt:YUYV8_1X16/1920x1080 field:none colorspace:srgb]"
			media-ctl -d "${media_dev}" -V "'seninf-$idx':2 [fmt:YUYV8_1X16/1920x1080 field:none colorspace:srgb]"
			media-ctl -d "${media_dev}" -V "'seninf-$idx':3 [fmt:YUYV8_1X16/1920x1080 field:none colorspace:srgb]"
			media-ctl -d "${media_dev}" -V "'seninf-$idx':4 [fmt:YUYV8_1X16/1920x1080 field:none colorspace:srgb]"
			if [ $idx -eq 1 ]; then
				media-ctl -d "${media_dev}" -l "'seninf-$idx':1 -> 'mtk-cam camsv-0':0 [5]"
				media-ctl -d "${media_dev}" -l "'seninf-$idx':2 -> 'mtk-cam camsv-1':0 [5]"
				media-ctl -d "${media_dev}" -l "'seninf-$idx':3 -> 'mtk-cam camsv-2':0 [5]"
				media-ctl -d "${media_dev}" -l "'seninf-$idx':4 -> 'mtk-cam camsv-3':0 [5]"
				media-ctl -d "${media_dev}" -V "'mtk-cam camsv-0':0 [fmt:YUYV8_1X16/1920x1080 field:none colorspace:srgb]"
				media-ctl -d "${media_dev}" -V "'mtk-cam camsv-1':0 [fmt:YUYV8_1X16/1920x1080 field:none colorspace:srgb]"
				media-ctl -d "${media_dev}" -V "'mtk-cam camsv-2':0 [fmt:YUYV8_1X16/1920x1080 field:none colorspace:srgb]"
				media-ctl -d "${media_dev}" -V "'mtk-cam camsv-3':0 [fmt:YUYV8_1X16/1920x1080 field:none colorspace:srgb]"
				media-ctl -d "${media_dev}" -V "'mtk-cam camsv-0':1 [fmt:YUYV8_1X16/1920x1080 field:none colorspace:srgb]"
				media-ctl -d "${media_dev}" -V "'mtk-cam camsv-1':1 [fmt:YUYV8_1X16/1920x1080 field:none colorspace:srgb]"
				media-ctl -d "${media_dev}" -V "'mtk-cam camsv-2':1 [fmt:YUYV8_1X16/1920x1080 field:none colorspace:srgb]"
				media-ctl -d "${media_dev}" -V "'mtk-cam camsv-3':1 [fmt:YUYV8_1X16/1920x1080 field:none colorspace:srgb]"
			else
				media-ctl -d "${media_dev}" -l "'seninf-$idx':1 -> 'mtk-cam camsv-4':0 [5]"
				media-ctl -d "${media_dev}" -l "'seninf-$idx':2 -> 'mtk-cam camsv-5':0 [5]"
				media-ctl -d "${media_dev}" -V "'mtk-cam camsv-4':0 [fmt:YUYV8_1X16/1920x1080 field:none colorspace:srgb]"
				media-ctl -d "${media_dev}" -V "'mtk-cam camsv-5':0 [fmt:YUYV8_1X16/1920x1080 field:none colorspace:srgb]"
				media-ctl -d "${media_dev}" -V "'mtk-cam camsv-4':1 [fmt:YUYV8_1X16/1920x1080 field:none colorspace:srgb]"
				media-ctl -d "${media_dev}" -V "'mtk-cam camsv-5':1 [fmt:YUYV8_1X16/1920x1080 field:none colorspace:srgb]"
			fi
			(( idx-- ))
		done
	fi
}

run_camera()
{
	#Check it is bridge or not?  And is it out of renge
	if [ ${CSI_TYPE[$CAMID]} != "bridge" ] || [ $VCID -gt 3 ] || [ $VCID -lt 0 ]; then
		echo "Wrong CSI type(${CSI_TYPE[$CAMID]}) or VC id is out of range($VCID)"
		exit
	fi

	setup_camsv_media_links

	#Get the v4l2 video node index
	local dev_id=0
	local CSI_DEV=(`for i in {0..7}; do media-ctl -d ${MEDIA_DEV} --entity "mtk-cam camsv-$i main-stream"; done | tr "\n" " "`)
	if [ $BR_COUNT -eq 1 ]; then
		dev_id=$VCID
	elif [ $BR_COUNT -eq 2 ] && [ $CAMID -eq 1 ]; then
		dev_id=$VCID
	elif [ $BR_COUNT -eq 2 ] && [ $CAMID -eq 0 ]; then
		dev_id=$(($VCID+4))
	fi

	if [ $DEBUG -eq 1 ]; then
		echo "Open CSI port: $CAMID, name: ${CSI_NAME[$CAMID]}, type: ${CSI_TYPE[$CAMID]}, VC id: $VCID"
		echo "V4l2 video idx: $dev_id, node: ${CSI_DEV[$dev_id]}"
	fi

	if [ "$MODE" == "preview" ]; then
		echo "device: ${CSI_DEV[$CAMID]} start preview"
		gst-launch-1.0 -e v4l2src device="${CSI_DEV[$dev_id]}" ! video/x-raw,width=${SEN_W},height=${SEN_H},format=UYVY \
			! v4l2convert output-io-mode=dmabuf-import ! video/x-raw,width=${WIDTH},height=${HEIGHT} \
			! fpsdisplaysink text-overlay="$FPS" sync=false video-sink="waylandsink sync=false"
	elif [ "$MODE" == "capture" ]; then
		echo "device: ${CSI_DEV[$CAMID]} start capture"
		SEN_W="1920"
		SEN_H="1080"
		FILE_NAME=$(echo "$CAMTYPE-camera-$CAMID-vc-$VCID-$MODE-1920x1080")
		gst-launch-1.0 -e v4l2src device="${CSI_DEV[$dev_id]}" num-buffers=1 ! video/x-raw,width=${SEN_W},height=${SEN_H},format=UYVY \
			! videoconvert ! v4l2jpegenc ! filesink location="$FILE_NAME.jpg"
	elif [ "$MODE" == "encode" ]; then
		echo "device: ${CSI_DEV[$dev_id]} start encode"
		FILE_NAME=$(echo "$CAMTYPE-camera-$CAMID-vc-$VCID-$MODE-${SEN_W}x${SEN_H}")
		gst-launch-1.0 -e v4l2src device="${CSI_DEV[$dev_id]}" ! video/x-raw,width=${SEN_W},height=${SEN_H},format=UYVY \
			! tee name=usbcam  ! v4l2convert ! video/x-raw,colorimetry="2:4:5:1" ! $ENCSTRING ! queue ! mp4mux ! filesink location="$FILE_NAME.mp4" \
			usbcam. ! queue ! v4l2convert output-io-mode=dmabuf-import ! video/x-raw,width=${WIDTH},height=${HEIGHT} \
			! fpsdisplaysink text-overlay="$FPS" sync=false video-sink="waylandsink sync=false"
	else
		echo "unsupported mode: $mode"
		return
	fi
}

run_csi_camera()
{
	local videodev=""

	echo "CSI port: $CAMID, VC id: $VCID, mode: $MODE, width: $WIDTH, height: $HEIGHT"

	if [ "${CSI_TYPE[$CAMID]}" == "bridge" ]; then
		run_camera
		return
	elif [ "${CSI_TYPE[$CAMID]}" == "disable" ]; then
		return
	fi

	#Check Cam id is exist
	videodev=(`v4l2-ctl --list-devices | grep -P "mtk-v4l2-camera.*${CSI_NAME[$CAMID]}" -A 3 | grep -P "video" | tr -d "\n"`)
	if [ "${videodev[0]}" == "" ]; then
		echo "Can not find CSI port: $CSI_PORT, (${CSI_NAME[$CAMID]})"
		return
	fi

	FILE_NAME=$(echo "$CAMTYPE-camera-$CSI_PORT-$MODE-${SEN_W}x${SEN_H}")

	if [ "$MODE" == "preview" ]; then
		echo "device: ${videodev[0]}"
		gst-launch-1.0 -e v4l2src device="${videodev[0]}" ! video/x-raw,width=${SEN_W},height=${SEN_H},format=YUY2 \
			! v4l2convert ! video/x-raw,width=${WIDTH},height=${HEIGHT} \
			! fpsdisplaysink text-overlay="$FPS" sync=false video-sink="waylandsink sync=false"
	elif [ "$MODE" == "capture" ]; then
		echo "device: ${videodev[2]}"
		get_resolution ${videodev[2]} 0 "JFIF" 10
	        echo "Get SEN_W: $SEN_W, SEN_H: $SEN_H"
		FILE_NAME=$(echo "$CAMTYPE-camera-$CAMID-$MODE-${SEN_W}x${SEN_H}")
		gst-launch-1.0 -e v4l2src device="${videodev[2]}" num-buffers=1 ! image/jpeg,width=${SEN_W},height=${SEN_H},format=JPEG \
			! filesink location="$FILE_NAME.jpg"
	elif [ "$MODE" == "encode" ]; then
		echo "device: ${videodev[1]}"
		gst-launch-1.0 -e v4l2src device="${videodev[1]}" ! video/x-raw,width=${SEN_W},height=${SEN_H},format=NV12 \
			! fpsdisplaysink text-overlay="$FPS" sync=false \
			video-sink="$ENCSTRING ! queue ! mp4mux ! filesink location=\"$FILE_NAME.mp4\""
	else
		echo "unsupported mode: $MODE"
		return
	fi

	sync
}

run_ahd_camera()
{
	#This function just for vab-5000, when input parameter "-t ahd"
	local platform=`get_board_name`
	if [[ "$platform" != "vab-5000" ]]; then
		echo "This device($platform) not support RN6752"
		exit
	fi
	#echo "RN6752 mode: $MODE, width: $WIDTH, height: $height"

	#export MEDIA_DEV=$(v4l2-ctl -z platform:$(basename `find /sys/bus/platform/devices/ -name "*camisp"`) --list-devices | grep media)
	echo ${MEDIA_DEV}
	media-ctl -d ${MEDIA_DEV} -l "'seninf-0':1 -> 'mtk-cam camsv-0':0 [5]"
	media-ctl -d ${MEDIA_DEV} -l "'rn6752.5-002c':0 -> 'seninf-0':0 [1]"
	media-ctl -d ${MEDIA_DEV} -V "'rn6752.5-002c':0 [fmt:YUYV8_1X16/1920x1080 field:none]"
	media-ctl -d ${MEDIA_DEV} -V "'seninf-0':1 [fmt:YUYV8_1X16/1920x1080 field:none]"
	media-ctl -d ${MEDIA_DEV} -V "'mtk-cam camsv-0':1 [fmt:YUYV8_1X16/1920x1080 field:none]"
	declare -a VIDEO_DEV=(`for i in {0..5}; do media-ctl -d ${MEDIA_DEV} --entity "mtk-cam camsv-$i main-stream"; done | tr "\n" " "`)
	#echo ${VIDEO_DEV[*]}

	if [ "$VIDEO_DEV[0]" == "" ]; then
		echo "Can not find AHD DEVICES"
		exit
	fi

	if [ "$MODE" == "preview" ]; then
		echo "device: ${VIDEO_DEV[0]} start preview"
		gst-launch-1.0 -e v4l2src device="${VIDEO_DEV[0]}" ! video/x-raw,width=${SEN_W},height=${SEN_H},format=UYVY \
			! v4l2convert output-io-mode=dmabuf-import ! video/x-raw,width=${WIDTH},height=${HEIGHT} \
			! fpsdisplaysink text-overlay="$FPS" sync=false video-sink="waylandsink sync=false"
	elif [ "$MODE" == "capture" ]; then
		echo "device: ${VIDEO_DEV[0]} start capture"
		SEN_W="1920"
		SEN_H="1080"
		FILE_NAME=$(echo "$CAMTYPE-camera-$CAMID-$MODE-1920x1080")
		gst-launch-1.0 -e v4l2src device="${VIDEO_DEV[0]}" num-buffers=1 ! video/x-raw,width=${SEN_W},height=${SEN_H},format=UYVY \
			! videoconvert ! v4l2jpegenc ! filesink location="$FILE_NAME.jpg"
	elif [ "$MODE" == "encode" ]; then
		echo "device: ${VIDEO_DEV[0]} start encode"
		 FILE_NAME=$(echo "$CAMTYPE-camera-$CAMID-$MODE-${SEN_W}x${SEN_H}")
		gst-launch-1.0 -e v4l2src device="${VIDEO_DEV[0]}" ! video/x-raw,width=${SEN_W},height=${SEN_H},format=UYVY \
			! tee name=usbcam  ! v4l2convert ! video/x-raw,colorimetry="2:4:5:1" ! $ENCSTRING ! queue ! mp4mux ! filesink location="$FILE_NAME.mp4" \
			usbcam. ! queue ! v4l2convert output-io-mode=dmabuf-import ! video/x-raw,width=${WIDTH},height=${HEIGHT} \
			! fpsdisplaysink text-overlay="$FPS" sync=false video-sink="waylandsink sync=false"
	else
		echo "unsupported mode: $mode"
		return
	fi

	sync
}

run_usb_camera()
{

	local videodev=""

	echo "USB Camera: camid: $CAMID, fmt: $FMT, mode: $MODE, width: $WIDTH, height: $HEIGHT"

	list_usb_camera 0

	if [ "${UVC_DEV[$CAMID]}" == "" ]; then
		echo "Can not find USB Camera for cam id$CAMID"
		return
	fi

	if [ "$MODE" == "capture" ]; then
		get_resolution ${UVC_DEV[$CAMID]} 0 "Motion-JPEG" 1000
	else
		SEN_W="1280"
		SEN_H="720"
	fi

	FILE_NAME=$(echo "$CAMTYPE-camera-$CAMID-$MODE-${SEN_W}x${SEN_H}")

	if [ "$MODE" == "preview" ]; then
		echo "device: ${UVC_DEV[$CAMID]}"
		gst-launch-1.0 -e v4l2src device="${UVC_DEV[$CAMID]}" ! image/jpeg,width=${SEN_W},height=${SEN_H},framerate=30/1 \
			! jpegdec ! v4l2convert ! video/x-raw,width=${WIDTH},height=${HEIGHT} \
			! fpsdisplaysink text-overlay="$FPS" sync=false video-sink="waylandsink sync=false"
	elif [ "$MODE" == "capture" ]; then
		echo "device: ${UVC_DEV[$CAMID]}, SEN_W: $SEN_W, SEN_H: $SEN_H"
		gst-launch-1.0 -e v4l2src device="${UVC_DEV[$CAMID]}" num-buffers=1 ! image/jpeg,width=${SEN_W},height=${SEN_H},format=JPEG \
			! jpegdec ! videoconvert ! v4l2jpegenc ! filesink location="$FILE_NAME.jpg"
	elif [ "$MODE" == "encode" ]; then
		echo "device: ${UVC_DEV[$CAMID]}, SEN_W: $SEN_W, SEN_H: $SEN_H for 30 fps"
		gst-launch-1.0 -e v4l2src device="${UVC_DEV[$CAMID]}" ! image/jpeg,width=${SEN_W},height=${SEN_H} ! jpegdec \
			! tee name=usbcam  ! queue ! v4l2convert ! video/x-raw,colorimetry="1:4:7:1" ! $ENCSTRING ! queue ! mp4mux ! filesink location="$FILE_NAME.mp4" \
			usbcam. ! queue ! v4l2convert ! video/x-raw,width=${WIDTH},height=${HEIGHT} \
			! fpsdisplaysink text-overlay="$FPS" sync=false video-sink="waylandsink sync=false"
	else
		echo "unsupported mode: $MODE"
		return
	fi

	sync
}

parse_camera_port()
{
	CAMID=$(echo $1 | tr -t ":" " "| awk {'print $1'})
	if [ "${CSI_TYPE[$CAMID]}" = "bridge" ]; then
		VCID=$(echo $1 | tr -t ":" " " | awk {'print $2'})
		if [ "$VCID" == "" ]; then
			VCID=0
		fi
	else
		VCID=0
		echo "mtkcam is not support virtual channel"
	fi
	if [ $DEBUG -eq 1 ]; then
		echo "CSI_PORT:$CAMID, VC_ID:$VCID"
	fi
}

parameter_check()
{
    case $1 in
        -t | -c | -m | -f | -r | -e)
            PARA1=1
            ;;
        *)
            PARA1=0
            ;;
    esac
    
    case $2 in
        -t | -c | -m | -f | -r | -e | --list | --help)
            PARA2=1
            ;;
        *)
            PARA2=0
            ;;
    esac
    
    if [ $PARA1 -eq 1 ] && [ $PARA2 -eq 0 ]; then
        return 1 #Get parameter
    else
        return 0 #Need shift
    fi
}

INDEX=0
N_PARA=$#

echo "Start run vcamera (version: $VERSION)"
find_csi_port_dev
list_csi_camera

while [ $INDEX -lt $N_PARA ]
do
	if [ "$1" == "--list" ]; then
		echo "LIST CAMERA:"
		#list_csi_camera 0
		list_usb_camera 0
		exit
	elif [ "$1" == "--help" ]; then
		vcamera_help
		exit
	fi

	parameter_check $1 $2

	if [ $? -eq 1 ] && [ "$2" != "" ]; then
	case $1 in
		-t)
			CAMTYPE=$2
    			(( INDEX++ ))
			echo "Change cam type to $CAMTYPE"
			shift
			;;
		-c)
			#CAMID=$2
			parse_camera_port $2
			#echo "Change cam id to $CAMID"
			(( INDEX++ ))
			shift
			;;
		-m)
			MODE=$2
			echo "Change mode to $MODE"
			(( INDEX++ ))
			shift
			;;
		-r)
			TMP=$(echo $2 | tr "x" " "| awk {'print $1'})
			if [ "$TMP" != "" ]; then
				WIDTH=$TMP
			fi
			TMP=$(echo $2 | tr "x" " "| awk {'print $2'})
			if [ "$TMP" != "" ]; then
				HEIGHT=$TMP
			fi
			echo "Change resolution to ${WIDTH}x${HEIGHT}"
			(( INDEX++ ))
			shift
			;;
		-e)
 			ENCTYPE=$2
 			if [ "$ENCTYPE" == "h265" ]; then
				ENCSTRING=$H265_ENCODE
			else
				ENCSTRING=$H264_ENCODE
			fi
			echo "Change encode type to $ENCTYPE"
			(( INDEX++ ))
			shift
			;;
		*) 	echo "unsupport parameter"
	esac
	fi
	shift
	(( INDEX++ ))
done

echo "cam type: $CAMTYPE, csi port: $CAMID, vc id:$VCID, mode: $MODE, display resolution: ${WIDTH}x${HEIGHT}, codec: $ENCTYPE"

if [ "$CAMTYPE" == "csi" ]; then
	#echo "Start open CSI camera"
	run_csi_camera
elif [ "$CAMTYPE" == "ahd" ]; then
	#echo "Start open AHD camera"
	run_ahd_camera
elif [ "$CAMTYPE" == "usb" ]; then
	#echo "Start open USB camera"
	run_usb_camera
else
	echo "unsupported camera type: $CAMTYPE"
fi

