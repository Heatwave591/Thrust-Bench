import pygame
import serial

# Initialize serial connection
ser = serial.Serial(port='COM3', baudrate=9600)
active_entry = (0, 0)  # Initialize active_entry to a default value

# Initialize Pygame
pygame.init()

# Create game window
SCREEN_WIDTH = 1520
SCREEN_HEIGHT = 770

throttle_entry = ''
entry_box_x = SCREEN_WIDTH - 300  # Define entry_box_x in the global scope
screen = pygame.display.set_mode((SCREEN_WIDTH, SCREEN_HEIGHT))
pygame.display.set_caption("Sensor Data Visualization")


# Define fonts and colors
font = pygame.font.SysFont("arialblack", 24)
TEXT_COL = (255, 255, 255)
BOX_COLOR = (150, 150, 50)
ENTRY_COLOR = (150, 150, 150)
ACTIVE_ENTRY_COLOR = (200, 200, 200)

# Define button coordinates
HALT_BUTTON_X = SCREEN_WIDTH - 180
HALT_BUTTON_Y = SCREEN_HEIGHT - 180
HALT_BUTTON_RADIUS = 80

# Game variables
menu_state = "home"
receive_data = True
data_values = {"Voltage": 0, "Current": 0, "Torque1": 0, "Torque2": 0, "Thrust": 0}

# Entry box parameters
entry_width = 100
entry_height = 40
entry_x = [700, 1100]  # X positions for entry boxes
entry_y = [200, 300, 400]  # Y positions for entry boxes
entry_text = [["", ""], ["", ""], ["", ""]]  # Initial text for entry boxes


# Entry box labels
entry_labels = [["Voltage Min", "Voltage Max"],
                ["Current Min", "Current Max"],
                ["Torque1 Min", "Torque1 Max"]]

def draw_set_throttle_button():
    button_x = 1170  # Adjusted x-coordinate for the button
    button_y = 125
    pygame.draw.rect(screen, ENTRY_COLOR, (button_x, button_y, entry_width + 100, entry_height), 2)
    draw_text("Set Throttle", font, TEXT_COL, button_x + 10, button_y + 10)

def draw_general_data():
    data_y = 200
    for key, value in data_values.items():
        text = f"{key}: {value}"
        draw_text(text, font, TEXT_COL, 400, data_y)
        data_y += 100

    draw_throttle_entry()
def handle_general_menu_event(event):
    global throttle_entry
    if event.type == pygame.KEYDOWN:
        if event.key == pygame.K_BACKSPACE:
            throttle_entry = throttle_entry[:-1]  # Remove last character
        elif event.key == pygame.K_RETURN:
            try:
                float_value = float(throttle_entry)  # Convert the entered text to float
                throttle_entry = str(float_value)  # Update the throttle entry with the valid float
                print(f"Current custom entry box: {throttle_entry}")
            except ValueError:
                print("Invalid input. Please enter a valid float value.")
                throttle_entry = ""  # Clear the entry box if not a valid float
        else:
            # Add typed character to entry text only if it results in a valid float
            try:
                float(throttle_entry + event.unicode)
                throttle_entry += event.unicode
            except ValueError:
                print("Invalid input. Please enter a valid float value.")


def draw_throttle_entry():
    entry_box_x = SCREEN_WIDTH - 300
    pygame.draw.rect(screen, ENTRY_COLOR, (entry_box_x, 50 + 20, entry_width, entry_height), 2)
    draw_text("Throttle", font, TEXT_COL, entry_box_x - 120, 50 + 30)
    
    # Display the latest throttle value from the data_values dictionary
    # draw_text(f"Throttle Value: {data_values['Throttle']}", font, TEXT_COL, entry_box_x + 10, 50 + 150)


# Function to draw text on the screen
def draw_text(text, font, text_col, x, y):
    img = font.render(text, True, text_col)
    screen.blit(img, (x, y))


# Function to read and process serial data
def read_serial_data():
    global receive_data, data_values
    try:
        if ser.in_waiting > 0 and receive_data:
            data = ser.readline().decode('UTF-8').strip()
            values = [float(value) for value in data.split("\t") if value.strip()]
            if len(values) == 5:
                data_values["Voltage"], data_values["Current"], data_values["Torque1"], data_values["Torque2"], data_values["Thrust"] = values
    except ValueError as e:
        print("Error processing serial data:", e)


# Function to draw the menu
def draw_menu():
    menu_items = ["General", "Limits", "Graph"]
    menu_y = 50
    box_width = 200
    box_height = 70
    for item in menu_items:
        pygame.draw.rect(screen, BOX_COLOR, (50, menu_y, box_width, box_height), 2)
        draw_text(item, font, TEXT_COL, 50 + 10, menu_y + 10)
        menu_y += 100


# Function to draw data in the General section
def draw_general_data():
    data_y = 200  # Adjusted y-coordinate to move the entry box up
    data_y1 = 50
    for key, value in data_values.items():
        text = f"{key}: {value}"
        draw_text(text, font, TEXT_COL, 400, data_y)
        data_y += 100

    # Add an entry box for a custom value on the right side
    entry_box_x = SCREEN_WIDTH - 300  # Adjusted x-coordinate for the right side
    pygame.draw.rect(screen, ENTRY_COLOR, (entry_box_x, data_y1 + 20, entry_width, entry_height), 2)
    draw_text("", font, TEXT_COL, entry_box_x - 200, data_y1 + 30)
    draw_text(entry_text[0][0], font, TEXT_COL, entry_box_x + 10, data_y1 + 30)


# Function to draw entry boxes and buttons in the Limits menu
def draw_limits_entry_boxes():
    for i in range(3):
        for j in range(2):
            color = ACTIVE_ENTRY_COLOR if (active_entry[0] == i and active_entry[1] == j) else (255, 255, 255)
            pygame.draw.rect(screen, color, (entry_x[j], entry_y[i], entry_width, entry_height), 2)
            draw_text(entry_labels[i][j], font, TEXT_COL, entry_x[j] - 200, entry_y[i] + 10)
            draw_text(entry_text[i][j], font, TEXT_COL, entry_x[j] + 10, entry_y[i] + 10)

    # Draw the "Send Data" button
    pygame.draw.rect(screen, BOX_COLOR, (50, 600, 200, 70), 2)
    draw_text("Send Data", font, TEXT_COL, 50 + 10, 600 + 10)


# Function to draw the HALT button
def draw_halt_button():
    pygame.draw.circle(screen, (255, 0, 0), (HALT_BUTTON_X, HALT_BUTTON_Y), HALT_BUTTON_RADIUS, 0)
    draw_text("HALT", font, TEXT_COL, HALT_BUTTON_X - 40, HALT_BUTTON_Y - 20)


def send_data_to_arduino():
    try:
        # Read values from entry boxes
        voltage_min = float(entry_text[0][0])
        voltage_max = float(entry_text[0][1])
        current_min = float(entry_text[1][0])
        current_max = float(entry_text[1][1])
        torque1_min = float(entry_text[2][0])
        torque1_max = float(entry_text[2][1])

        # Format data and send it to Arduino
        data_to_send = f"{voltage_min}\t{voltage_max}\t{current_min}\t{current_max}\t{torque1_min}\t{torque1_max}\n"
        ser.write(data_to_send.encode())

        print("Data sent to Arduino:", data_to_send)
    except ValueError:
        print("Error: Invalid input. Please enter valid float values in all entry boxes.")

def draw_throttle_entry():
    entry_box_x = SCREEN_WIDTH - 300
    pygame.draw.rect(screen, ENTRY_COLOR, (entry_box_x, 50 + 20, entry_width, entry_height), 2)
    draw_text("Throttle", font, TEXT_COL, entry_box_x - 120, 50 + 30)
    draw_text(throttle_entry, font, TEXT_COL, entry_box_x + 10, 50 + 30)

    # Add this line to display the throttle value
    draw_text(f"Throttle Value: {throttle_entry}", font, TEXT_COL, 1150, 50 + 120   )

def send_throttle_to_arduino(throttle_value):
    try:
        # Format data and send it to Arduino
        data_to_send = f"SET_THROTTLE\t{throttle_value}\n"
        ser.write(data_to_send.encode())
        print("Throttle Data sent to Arduino:", data_to_send)

        # Clear the entry box after sending the throttle value
        global throttle_entry
        throttle_entry = ""
        # Update the data_values dictionary to store the latest throttle value
        data_values["Throttle"] = throttle_value
    except ValueError:
        print("Error: Invalid throttle input.")

def handle_click(x, y):
    global receive_data, menu_state, active_entry, entry_box_x

    if HALT_BUTTON_X - HALT_BUTTON_RADIUS <= x <= HALT_BUTTON_X + HALT_BUTTON_RADIUS and \
            HALT_BUTTON_Y - HALT_BUTTON_RADIUS <= y <= HALT_BUTTON_Y + HALT_BUTTON_RADIUS:
        print("Clicked on HALT button")
        receive_data = False  # Stop receiving data
        ser.write("HALT".encode())

    elif 50 <= x <= 250:
        if 50 <= y <= 120:  # Check if clicked on "Limits" button
            menu_state = "home"
            active_entry = (0, 0)  # Set the first entry box as active
            print("Clicked on Limits")
        elif 150 <= y <= 220:  # Check if clicked on "General" button within the "Limits" menu
            menu_state = "limits"
            print("Clicked on General")
        elif 600 <= y <= 670 and menu_state == "home":  # Check if clicked on "Send Data" button in the "General" menu
            print("Clicked on Send Data")
            send_data_to_arduino()

    button_x = 1170
    button_y = 125
    if button_x <= x <= button_x + entry_width and button_y <= y <= button_y + entry_height:
        print("Clicked on Set Throttle button")
        try:
            throttle_value = float(throttle_entry)
            send_throttle_to_arduino(throttle_value)  # Call the function to send the throttle value to Arduino
            # Perform any necessary actions with the throttle value
            print(f"Setting throttle to: {throttle_value}")
        except ValueError:
            print("Invalid throttle value. Please enter a valid float.")


    for i in range(3):
        for j in range(2):
            entry_x_min = entry_x[j]
            entry_x_max = entry_x[j] + entry_width
            entry_y_min = entry_y[i]
            entry_y_max = entry_y[i] + entry_height
            if entry_x_min <= x <= entry_x_max and entry_y_min <= y <= entry_y_max:
                active_entry = (i, j)
                print("Clicked on entry box:", active_entry)

                
# Function to handle entry box events
# Define variables to store the entries
voltage_min = 0.0
voltage_max = 0.0
current_min = 0.0
current_max = 0.0
param3_min = 0.0
param3_max = 0.0

# Function to handle entry box events
def handle_entry_event(event):
    global active_entry
    if event.type == pygame.KEYDOWN:
        i, j = active_entry
        if event.key == pygame.K_BACKSPACE:
            entry_text[i][j] = entry_text[i][j][:-1]  # Remove the last character
        elif event.key == pygame.K_RETURN:
            try:
                float_value = float(entry_text[i][j])  # Convert the entered text to float
                if i == 0 and j == 0:
                    voltage_min = float_value
                elif i == 0 and j == 1:
                    voltage_max = float_value
                elif i == 1 and j == 0:
                    current_min = float_value
                elif i == 1 and j == 1:
                    current_max = float_value
                elif i == 2 and j == 0:
                    param3_min = float_value
                elif i == 2 and j == 1:
                    param3_max = float_value
                print(f"Value entered in entry box [{i}][{j}]: {float_value}")
            except ValueError:
                print("Invalid input. Please enter a valid float value.")
                entry_text[i][j] = ""  # Clear the entry box if not a valid float
        else:
            entry_text[i][j] += event.unicode  # Add typed character to entry text
            print(f"Current entry box [{i}][{j}]: {entry_text[i][j]}")
        # Update the active entry to the entry currently receiving keyboard input
        active_entry = (i, j)

run = True
# clock = pygame.time.Clock()
while run:
    screen.fill((52, 78, 91))

    # Handle events
    for event in pygame.event.get():
        if event.type == pygame.MOUSEBUTTONDOWN:
            if event.button == 1:  # Left mouse button
                x, y = event.pos
                handle_click(x, y)
        if event.type == pygame.KEYDOWN:
            if menu_state == "home":
                handle_general_menu_event(event)
            elif menu_state == "limits":
                handle_entry_event(event)
        if event.type == pygame.QUIT:
            run = False

    # Read serial data
    read_serial_data()
    draw_halt_button() 
    # Draw functions
    if menu_state == "home":
        draw_general_data()
        draw_set_throttle_button()
        draw_throttle_entry() # <-- Add this line to update the throttle entry box
    elif menu_state == "limits":
        draw_limits_entry_boxes()  # Call the function to draw entry boxes and buttons in the "Limits" menu

    draw_menu()

    pygame.display.update()
    # clock.tick(60)  # Limit frame rate to 30 FPS

pygame.quit()
