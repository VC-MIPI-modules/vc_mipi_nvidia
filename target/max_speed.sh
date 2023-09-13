#!/bin/sh

path=/sys/kernel/debug/bpmp/debug/clk

echo 1 > ${path}/vi/mrq_rate_locked
echo 1 > ${path}/isp/mrq_rate_locked
echo 1 > ${path}/nvcsi/mrq_rate_locked
echo 1 > ${path}/emc/mrq_rate_locked
cat ${path}/vi/max_rate | tee ${path}/vi/rate
cat ${path}/isp/max_rate | tee  ${path}/isp/rate
cat ${path}/nvcsi/max_rate | tee ${path}/nvcsi/rate
cat ${path}/emc/max_rate | tee ${path}/emc/rate