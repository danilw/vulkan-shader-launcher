#!/bin/sh

VULKAN_SDK="<path to Vulkan SDK/version>" #example /home/user/vulkan_sdk/1.2.131.2/
VK_APP=$1

## bash and dash
if which source >/dev/null; then 
	source $VULKAN_SDK"/setup-env.sh"
else
	. $VULKAN_SDK"/setup-env.sh"
fi
export VK_INSTANCE_LAYERS=VK_LAYER_KHRONOS_validation
./$VK_APP
