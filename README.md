I will be the first to admit that this is a pretty frivilous project.  It is an Audio/Visual mix of Thanksgiving and Haloween.
It's real purpose was to develop functions for:
   Playing Mp3 files through an I2S DAC.
   Playing text strings through an I2S DAC.
   Generating Text messages by OpenAI from text prompts.
   Displaying PNG files from a web resource.
   
The playing of audio files uses the  OpenAT TTS API,  It is very inexpensive but not free. The generation of text from the API also involves a small per use fee.

A large amout of debugging of this project was done by Microsft Project.

Thsi project must use an ESP32S3 (Huge memory model) with PSRAM.  I have not tested it with different amounts of PSRAM so I am not sure how much is needed.  I used 8M.


<img width="734" height="624" alt="Screenshot 2025-10-28 064733" src="https://github.com/user-attachments/assets/d5d2075d-107d-464d-a376-a3fe4f4afab1" />
