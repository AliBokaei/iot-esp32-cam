import requests
from telegram import Update
from telegram.ext import ApplicationBuilder, CommandHandler, MessageHandler, filters, ContextTypes

# Esp23 URL
URL = '***'

# Dictionary to store user states
user_states = {}


async def send_req(command, update: Update, led_on_time=None, led_off_time=None):
    try:
        with requests.Session() as s:
            if command == 'on':
                response = s.post(f"{URL}/?command=on", timeout=60)
                if response.status_code == 200:
                    await update.message.reply_text("The light is now on.")
                else:
                    await update.message.reply_text("Failed to turn on the light.")
            elif command == 'off':
                response = s.post(f"{URL}/?command=off", timeout=60)
                if response.status_code == 200:
                    await update.message.reply_text("The light is now off.")
                else:
                    await update.message.reply_text("Failed to turn off the light.")
            elif command == 'led_state':
                response = s.post(f"{URL}/?ledState", timeout=60)
                print(response)
                if response.status_code == 200:
                    await update.message.reply_text("{response}")
                    print(response.text)
                    print(response)
                else:
                    await update.message.reply_text("Failed to turn off the light.")
            elif command == 'time':
                response = s.post(f"{URL}/?ledOnTime={led_on_time}&ledOffTime={led_off_time}", timeout=60)
                if response.status_code == 200:
                    await update.message.reply_text(f"LED times set: on - {led_on_time} ms, off - {led_off_time} ms.")
                else:
                    await update.message.reply_text("Failed to set LED times.")
    except requests.exceptions.RequestException as e:
        await update.message.reply_text(str(e))


async def start(update: Update, context: ContextTypes.DEFAULT_TYPE) -> None:
    await update.message.reply_text(f'Welcome to the esp controller {update.effective_user.first_name}!')


async def help(update: Update, context: ContextTypes.DEFAULT_TYPE) -> None:
    await update.message.reply_text('Commands:\n'
                                    '/on - Turn on the light.\n'
                                    '/off - Turn off the light.\n'
                                    '/time - Set LED on and off times.\n')


async def on_command(update: Update, context: ContextTypes.DEFAULT_TYPE) -> None:
    await send_req('on', update)


async def off_command(update: Update, context: ContextTypes.DEFAULT_TYPE) -> None:
    await send_req('off', update)


async def time_command(update: Update, context: ContextTypes.DEFAULT_TYPE) -> None:
    user_states[update.effective_user.id] = 'waiting_for_led_on_time'
    await update.message.reply_text("Please enter the LED on time in milliseconds:")


async def handle_message(update: Update, context: ContextTypes.DEFAULT_TYPE) -> None:
    user_id = update.effective_user.id

    if user_id in user_states:
        state = user_states[user_id]

        if state == 'waiting_for_led_on_time':
            try:
                led_on_time = int(update.message.text)
                user_states[user_id] = ('waiting_for_led_off_time', led_on_time)
                await update.message.reply_text("Please enter the LED off time in milliseconds:")
            except ValueError:
                await update.message.reply_text("Invalid input. Please enter a valid number for the LED on time.")

        elif state[0] == 'waiting_for_led_off_time':
            try:
                led_off_time = int(update.message.text)
                led_on_time = state[1]
                await send_req('time', update, led_on_time, led_off_time)
                del user_states[user_id]
            except ValueError:
                await update.message.reply_text("Invalid input. Please enter a valid number for the LED off time.")


if __name__ == '__main__':

    # Your Token
    app = ApplicationBuilder().token("***").build() 

    app.add_handler(CommandHandler("start", start))
    app.add_handler(CommandHandler("help", help))
    app.add_handler(CommandHandler("on", on_command))
    app.add_handler(CommandHandler("off", off_command))
    app.add_handler(CommandHandler("time", time_command))
    app.add_handler(MessageHandler(filters.TEXT & ~filters.COMMAND, handle_message))

    app.run_polling()
